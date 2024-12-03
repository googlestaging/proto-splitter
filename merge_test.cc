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
#include "third_party/proto_splitter/merge.h"

#include <fcntl.h>

#include <array>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "absl/strings/str_cat.h"
#include "proto_splitter/cc/test_util.h"
#include "proto_splitter/cc/util.h"
#include "proto_splitter/chunk.proto.h"
#include "proto_splitter/testdata/test_message.proto.h"
#include "protobuf/io/zero_copy_stream_impl.h"
#include "protobuf/message.h"
#include "protobuf/text_format.h"

namespace proto_splitter {

namespace {

using ::absl::StrCat;
using ::proto2::Message;
using ::proto2::TextFormat;
using ::proto2::io::FileInputStream;
using ::proto_splitter::ChunkedMessage;
using ::proto_splitter_testdata::ManyFields;
using ::proto_splitter_testdata::StringNode;
using std::filesystem::path;

inline constexpr std::array kDFSplitTreeChunks = {
    "val: \"0\"",       "val: \"010\"",     "val: \"01020\"",
    "val: \"0102030\"", "val: \"0102031\"", "val: \"0102032\"",
    "val: \"01021\"",   "val: \"0102130\"", "val: \"0102131\"",
    "val: \"0102132\""};

inline constexpr std::array kBFSplitTreeChunks = {
    "val: \"0\"",       "val: \"010\"",     "val: \"01020\"",
    "val: \"01021\"",   "val: \"0102030\"", "val: \"0102031\"",
    "val: \"0102032\"", "val: \"0102130\"", "val: \"0102131\"",
    "val: \"0102132\""};

inline path TestdataPath() {
  return path(proto_splitter::SrcRoot()) / "testdata";
}

TEST(MergeTest, TestReadRiegeliTreeDepthFirst) {
  StringNode merged_tree;
  ASSERT_OK(Merger::Read(path(TestdataPath() / "df-split-tree"), &merged_tree));

  int fd = open(path(TestdataPath() / "split-tree.pbtxt").c_str(), O_RDONLY);
  ASSERT_NE(fd, -1);

  FileInputStream fis(fd);
  StringNode test_proto;
  ASSERT_TRUE(TextFormat::Parse(&fis, &test_proto));

  EXPECT_THAT(merged_tree, EqualsProto(test_proto));
}

TEST(MergeTest, TestReadRiegeliTreeBreadthFirst) {
  StringNode merged_tree;
  ASSERT_OK(Merger::Read(path(TestdataPath() / "bf-split-tree"), &merged_tree));

  int fd = open(path(TestdataPath() / "split-tree.pbtxt").c_str(), O_RDONLY);
  ASSERT_NE(fd, -1);

  FileInputStream fis(fd);
  StringNode test_proto;
  ASSERT_TRUE(TextFormat::Parse(&fis, &test_proto));

  EXPECT_THAT(merged_tree, EqualsProto(test_proto));
}

TEST(MergeTest, TestMergeTreeChunksDepthFirst) {
  std::vector<std::unique_ptr<Message>> chunks;
  for (const auto& chunk : kDFSplitTreeChunks) {
    StringNode string_node;
    TextFormat::ParseFromString(chunk, &string_node);
    std::unique_ptr<Message> node = std::make_unique<StringNode>(string_node);
    chunks.push_back(std::move(node));
  }

  int fd = open(path(TestdataPath() / "df-split-tree.pbtxt").c_str(), O_RDONLY);
  ASSERT_NE(fd, -1);

  FileInputStream fis(fd);
  ChunkedMessage chunked_message;
  ASSERT_TRUE(TextFormat::Parse(&fis, &chunked_message));

  StringNode merged_tree;
  ASSERT_OK(Merger::Merge(chunks, chunked_message, &merged_tree));

  int fd2 = open(path(TestdataPath() / "split-tree.pbtxt").c_str(), O_RDONLY);
  ASSERT_NE(fd, -1);

  FileInputStream fis2(fd2);
  StringNode test_proto;
  ASSERT_TRUE(TextFormat::Parse(&fis2, &test_proto));

  EXPECT_THAT(merged_tree, EqualsProto(test_proto));
}

TEST(MergeTest, TestMergeTreeChunksBreadthFirst) {
  std::vector<std::unique_ptr<Message>> chunks;
  for (const auto& chunk : kBFSplitTreeChunks) {
    StringNode string_node;
    TextFormat::ParseFromString(chunk, &string_node);
    std::unique_ptr<Message> node = std::make_unique<StringNode>(string_node);
    chunks.push_back(std::move(node));
  }

  int fd = open(path(TestdataPath() / "bf-split-tree.pbtxt").c_str(), O_RDONLY);
  ASSERT_NE(fd, -1);

  FileInputStream fis(fd);
  ChunkedMessage chunked_message;
  ASSERT_TRUE(TextFormat::Parse(&fis, &chunked_message));

  StringNode merged_tree;
  ASSERT_OK(Merger::Merge(chunks, chunked_message, &merged_tree));

  int fd2 = open(path(TestdataPath() / "split-tree.pbtxt").c_str(), O_RDONLY);
  ASSERT_NE(fd, -1);

  FileInputStream fis2(fd2);
  StringNode test_proto;
  ASSERT_TRUE(TextFormat::Parse(&fis2, &test_proto));

  EXPECT_THAT(merged_tree, EqualsProto(test_proto));
}

TEST(MergeTest, TestReadManyField) {
  const std::string many_field_path = path(TestdataPath() / "many-field");
  ManyFields merged_many_field;
  ASSERT_OK(Merger::Read(many_field_path, &merged_many_field));

  int fd = open(StrCat(many_field_path, ".pbtxt").c_str(), O_RDONLY);
  ASSERT_NE(fd, -1);

  FileInputStream fis(fd);
  ManyFields test_many_field;
  ASSERT_TRUE(TextFormat::Parse(&fis, &test_many_field));

  EXPECT_THAT(merged_many_field, EqualsProto(test_many_field));
}

TEST(MergeTest, TestReadPartial) {
  const std::string many_field_path = path(TestdataPath() / "many-field");
  ASSERT_OK_AND_ASSIGN(auto reader, proto_splitter::GetRiegeliReader(
                                        StrCat(many_field_path, ".cpb")));

  auto read_metadata = GetChunkMetadata(reader);
  if (!read_metadata.ok()) {
    reader.Close();
    EXPECT_OK(read_metadata.status());
  }
  ChunkMetadata chunk_metadata = read_metadata.value();
  ChunkMetadata partial_chunk_metadata;
  partial_chunk_metadata.mutable_chunks()->CopyFrom(chunk_metadata.chunks());
  partial_chunk_metadata.mutable_message()->set_chunk_index(
      chunk_metadata.message().chunk_index());

  ManyFields merged_many_fields;
  ASSERT_OK(Merger::ReadPartial(many_field_path, partial_chunk_metadata,
                                &merged_many_fields));
  EXPECT_THAT(merged_many_fields, EqualsProto(R"pb(
                map_field_int64 { key: -1345 value: "map_value_-1345" }
              )pb"));
}

}  // namespace

}  // namespace proto_splitter
