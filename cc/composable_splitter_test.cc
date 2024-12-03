/* Copyright 2024 The Proto Splitter Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include "third_party/proto_splitter/cc/composable_splitter.h"

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "file/base/file.h"
#include "file/base/file_closer.h"
#include "file/base/filesystem.h"
#include "file/base/helpers.h"
#include "file/base/options.h"
#include "file/util/temp_file.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "third_party/absl/status/status.h"
#include "third_party/absl/strings/cord.h"
#include "third_party/absl/strings/str_cat.h"
#include "third_party/absl/strings/string_view.h"
#include "third_party/proto_splitter/cc/split.h"
#include "third_party/proto_splitter/cc/test_util.h"
#include "third_party/proto_splitter/cc/util.h"
#include "third_party/proto_splitter/chunk.proto.h"
#include "third_party/proto_splitter/testdata/test_message.proto.h"
#include "third_party/protobuf/message.h"
#include "third_party/riegeli/bytes/cord_reader.h"
#include "third_party/riegeli/bytes/file_reader.h"
#include "third_party/riegeli/bytes/string_reader.h"
#include "third_party/riegeli/records/record_reader.h"
#include "util/task/status_macros.h"

#define IS_OSS false

namespace proto_splitter {
namespace {

using ::proto_splitter::ChunkedMessage;
using ::proto_splitter::ChunkMetadata;
using ::proto_splitter::FieldType;
using ::proto_splitter::MessageBytes;
using ::proto_splitter_testdata::RepeatedRepeatedString;
using ::proto_splitter_testdata::RepeatedString;
using ::testing::HasSubstr;
using ::testing::SizeIs;
using ::testing::status::StatusIs;

// Required in OSS to prevent string to bool conversion in FieldType variant.
using namespace std::string_literals;  // NOLINT

// Splits each string in a RepeatedString into separate chunks.
class RepeatedStringSplitter : public ComposableSplitter {
  friend class ComposableSplitter;

 public:
  using ComposableSplitter::ComposableSplitter;

  absl::Status BuildChunks() override {
    RepeatedString* repeated_string =
        ::proto2::DynamicCastToGenerated<RepeatedString>(message());
    auto strings = repeated_string->strings();

    if (strings.empty()) {
      RETURN_IF_ERROR(SetMessageAsBaseChunk());
      return absl::OkStatus();
    }
    for (int i = 0; i < strings.size(); i++) {
      auto s = std::make_unique<MessageBytes>(strings[i]);
      std::vector<FieldType> fields = {"strings"s, i};
      RETURN_IF_ERROR(AddChunk(std::move(s), &fields));
    }
    return absl::OkStatus();
  }
};

RepeatedString SetUpRepeatedString(std::vector<std::string> strings) {
  RepeatedString message;
  *message.mutable_strings() = {strings.begin(), strings.end()};
  return message;
}

TEST(RepeatedStringSplitterTest, TestSplitChunks) {
  std::vector<std::string> strings = {"piece-1", "piece-2", "piece-3"};
  auto message = SetUpRepeatedString(strings);
  RepeatedStringSplitter splitter = RepeatedStringSplitter(&message);
  ASSERT_OK_AND_ASSIGN(auto ret, splitter.Split());
  auto chunks = ret.first;
  auto chunked_message = ret.second;

  for (int i = 0; i < chunks->size(); i++) {
    auto chunk = chunks->at(i);
    EXPECT_TRUE(std::holds_alternative<std::string>(chunk));
    EXPECT_EQ(strings[i], std::get<std::string>(chunk));
  }
  EXPECT_THAT(*chunked_message, EqualsProto(R"pb(chunked_fields {
                                                   field_tag { field: 1 }
                                                   field_tag { index: 0 }
                                                   message { chunk_index: 0 }
                                                 }
                                                 chunked_fields {
                                                   field_tag { field: 1 }
                                                   field_tag { index: 1 }
                                                   message { chunk_index: 1 }
                                                 }
                                                 chunked_fields {
                                                   field_tag { field: 1 }
                                                   field_tag { index: 2 }
                                                   message { chunk_index: 2 }
                                                 })pb"));

  // Calling split again should return the same chunks/ChunkedMessage.
  ASSERT_OK_AND_ASSIGN(auto ret2, splitter.Split());
  auto chunks2 = ret2.first;
  auto chunked_message2 = ret2.second;
  EXPECT_EQ(chunks2, chunks);
  EXPECT_EQ(chunked_message2, chunked_message);
}

static void CheckChunks(riegeli::RecordReaderBase& reader,
                        std::vector<std::string>& strings) {
  ChunkMetadata chunk_metadata;
  reader.Seek(reader.Size().value());
  reader.SeekBack();
  reader.ReadRecord(chunk_metadata);

  auto chunk_info = chunk_metadata.chunks();
  EXPECT_EQ(chunk_info.size(), strings.size());
  for (int i = 0; i < chunk_info.size(); i++) {
    reader.Seek(chunk_info[i].offset());
    absl::string_view chunk;
    reader.ReadRecord(chunk);
    EXPECT_EQ(strings[i], std::string(chunk));
  }

  EXPECT_THAT(chunk_metadata.message(),
              EqualsProto(R"pb(chunked_fields {
                                 field_tag { field: 1 }
                                 field_tag { index: 0 }
                                 message { chunk_index: 0 }
                               }
                               chunked_fields {
                                 field_tag { field: 1 }
                                 field_tag { index: 1 }
                                 message { chunk_index: 1 }
                               }
                               chunked_fields {
                                 field_tag { field: 1 }
                                 field_tag { index: 2 }
                                 message { chunk_index: 2 }
                               })pb"));
}

TEST(RepeatedStringSplitterTest, TestWrite) {
  std::vector<std::string> strings = {"piece-1", "piece-2", "piece-3"};
  auto message = SetUpRepeatedString(strings);
  RepeatedStringSplitter splitter = RepeatedStringSplitter(&message);

  ASSERT_OK_AND_ASSIGN(File * output_file,
                       file::CreateTempFile(::testing::TempDir()));
  FileCloser closer(output_file);
  ASSERT_OK(splitter.Write(output_file->filename()));
  std::string expected_file = absl::StrCat(output_file->filename(), ".cpb");

  ASSERT_OK(file::Exists(expected_file, file::Defaults()));

  // Look for the last chunk, which should contain a ChunkMetadata proto.
  riegeli::RecordReader file_reader{riegeli::FileReader(expected_file)};

  CheckChunks(file_reader, strings);
}

TEST(RepeatedStringSplitterTest, TestWriteToString) {
  std::vector<std::string> strings = {"piece-1", "piece-2", "piece-3"};
  auto message = SetUpRepeatedString(strings);
  RepeatedStringSplitter splitter = RepeatedStringSplitter(&message);
  auto string_output_results = splitter.WriteToString();
  EXPECT_OK(string_output_results.status());
  std::string string_output = std::get<0>(string_output_results.value());
  bool is_chunked = std::get<1>(string_output_results.value());
  EXPECT_TRUE(is_chunked);
  // Look for the last chunk, which should contain a ChunkMetadata proto.
  riegeli::RecordReader string_reader{riegeli::StringReader(string_output)};

  CheckChunks(string_reader, strings);
}

#if !IS_OSS
TEST(RepeatedStringSplitterTest, TestWriteToCord) {
  std::vector<std::string> strings = {"piece-1", "piece-2", "piece-3"};
  auto message = SetUpRepeatedString(strings);
  RepeatedStringSplitter splitter = RepeatedStringSplitter(&message);
  auto cord_output_results = splitter.WriteToCord();
  EXPECT_OK(cord_output_results.status());
  absl::Cord cord_output = std::get<0>(cord_output_results.value());
  bool is_chunked = std::get<1>(cord_output_results.value());
  EXPECT_TRUE(is_chunked);
  // Look for the last chunk, which should contain a ChunkMetadata proto.
  riegeli::RecordReader cord_reader{riegeli::CordReader(&cord_output)};

  CheckChunks(cord_reader, strings);
}
#endif

TEST(RepeatedStringSplitterTest, TestNoSplit) {
  RepeatedString message;  // No strings
  RepeatedStringSplitter splitter = RepeatedStringSplitter(&message);
  ASSERT_OK_AND_ASSIGN(auto ret, splitter.Split());
  auto chunks = ret.first;
  auto chunked_message = ret.second;

  EXPECT_THAT(*chunks, SizeIs(1));
  EXPECT_THAT(*std::get<::proto2::Message*>(chunks->at(0)), EqualsProto(""));
  EXPECT_THAT(*chunked_message, EqualsProto(R"pb(chunk_index: 0)pb"));
}

// Splits each string in a RepeatedString into separate chunks.
class RepeatedRepeatedStringSplitter : public ComposableSplitter {
 public:
  using ComposableSplitter::ComposableSplitter;

  absl::Status BuildChunks() override {
    RETURN_IF_ERROR(SetMessageAsBaseChunk());
    RepeatedRepeatedString* msg =
        ::proto2::DynamicCastToGenerated<RepeatedRepeatedString>(message());
    auto repeated_strings = msg->rs();
    for (int i = 0; i < repeated_strings.size(); i++) {
      std::vector<FieldType> fields = {"rs"s, i};
      auto splitter =
          RepeatedStringSplitter(&repeated_strings[i], this, &fields);
      RETURN_IF_ERROR(splitter.BuildChunks());
    }
    return absl::OkStatus();
  }
};

TEST(ComposableTest, RepeatedRepeatedStringTest) {
  std::vector<std::string> strings1 = {"piece-1", "piece-2", "piece-3"};
  auto rs1 = SetUpRepeatedString(strings1);
  std::vector<std::string> strings2 = {"new-strings-1"};
  auto rs2 = SetUpRepeatedString(strings2);
  std::vector<std::string> strings3 = {"foo-1", "foo-2"};
  auto rs3 = SetUpRepeatedString(strings3);

  std::vector<RepeatedString> rs = {rs1, rs2, rs3};

  RepeatedRepeatedString message;
  message.mutable_rs()->Add(rs.begin(), rs.end());

  RepeatedRepeatedStringSplitter splitter =
      RepeatedRepeatedStringSplitter(&message);
  ASSERT_OK_AND_ASSIGN(auto ret, splitter.Split());
  auto chunks = ret.first;
  auto chunked_message = ret.second;

  std::vector<std::string> expected_chunks = {
      "piece-1", "piece-2", "piece-3", "new-strings-1", "foo-1", "foo-2"};

  // RepeatedRepeatedStringSplitter sets the first chunk as the user-provided
  // message, so the expected size is 7.
  EXPECT_THAT(*chunks, SizeIs(7));
  EXPECT_THAT(*std::get<::proto2::Message*>(chunks->at(0)),
              EqualsProto(message));

  for (int i = 1; i < chunks->size(); i++) {
    auto chunk = chunks->at(i);
    EXPECT_TRUE(std::holds_alternative<std::string>(chunk));
    EXPECT_EQ(expected_chunks[i - 1], std::get<std::string>(chunk));
  }

  // message.rs[2].strings[0] (value = "foo-1") should be the chunk at index 5.
  EXPECT_THAT(chunked_message->chunked_fields()[4],
              EqualsProto(R"pb(field_tag { field: 2 }
                               field_tag { index: 2 }
                               field_tag { field: 1 }
                               field_tag { index: 0 }
                               message { chunk_index: 5 })pb"));
}

TEST(ComposableTest, ChildSplitterTest) {
  std::vector<std::string> strings1 = {"piece-1", "piece-2", "piece-3"};
  auto message1 = SetUpRepeatedString(strings1);
  RepeatedStringSplitter splitter(&message1);
  std::vector<FieldType> fields = {};

  std::vector<std::string> strings2 = {"s1", "s2"};
  auto message2 = SetUpRepeatedString(strings2);
  RepeatedStringSplitter child(&message2, &splitter, &fields);

  EXPECT_OK(child.BuildChunks());
  ASSERT_OK_AND_ASSIGN(auto ret, splitter.Split());
  auto chunks = ret.first;
  EXPECT_THAT(*chunks, SizeIs(5));  // Total 5 chunks should be generated.
}

TEST(ComposableTest, ChildSplitterUnimplementedTest) {
  RepeatedString message;
  RepeatedStringSplitter splitter(&message);
  std::vector<FieldType> fields = {};
  RepeatedStringSplitter child(&message, &splitter, &fields);

  EXPECT_THAT(child.Split(), StatusIs(absl::StatusCode::kUnimplemented,
                                      HasSubstr("`Split` function behavior")));
  EXPECT_THAT(child.Write("str"),
              StatusIs(absl::StatusCode::kUnimplemented,
                       HasSubstr("`Write` function behavior")));
}

class NoOpSplitter : public ComposableSplitter {
 public:
  using ComposableSplitter::ComposableSplitter;

  absl::Status BuildChunks() override { return absl::OkStatus(); }
};

TEST(NoOpSplitterTest, TestWrite) {
  std::vector<std::string> strings = {"piece-1", "piece-2", "piece-3"};
  auto message = SetUpRepeatedString(strings);
  NoOpSplitter splitter(&message);

  ASSERT_OK_AND_ASSIGN(File * output_file, file::CreateTempFile(""));
  FileCloser closer(output_file);
  ASSERT_OK(splitter.Write(output_file->filename()));
  std::string expected_file = absl::StrCat(output_file->filename(), ".pb");

  ASSERT_OK(file::Exists(expected_file, file::Defaults()));

  RepeatedString read_message;
  ASSERT_OK(
      file::GetBinaryProto(expected_file, &read_message, file::Defaults()));

  EXPECT_THAT(read_message, EqualsProto(message));
}

}  // namespace
}  // namespace proto_splitter
