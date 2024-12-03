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
#include "third_party/proto_splitter/cc/util.h"

#include <string>
#include <vector>

#include "file/base/path.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/gmock_utils/status-matchers.h"
#include "testing/base/public/gunit.h"
#include "third_party/absl/status/status.h"
#include "third_party/absl/status/statusor.h"
#include "third_party/proto_splitter/cc/test_util.h"
#include "third_party/proto_splitter/chunk.proto.h"
#include "third_party/proto_splitter/testdata/test_message.proto.h"
#include "third_party/protobuf/descriptor.h"
#include "third_party/protobuf/message.h"
#include "util/task/status_macros.h"

namespace proto_splitter {
namespace {

using ::proto_splitter::ChunkedField;
using ::proto_splitter::ChunkInfo;
using ::proto_splitter::ChunkMetadata;
using ::proto_splitter::FieldIndex;
using ::proto_splitter::FieldType;
using ::proto_splitter_testdata::ManyFields;
using ::testing::HasSubstr;
using ::testing::status::IsOkAndHolds;
using ::testing::status::StatusIs;

// Required in OSS to prevent string to bool conversion in FieldType variant.
using namespace std::string_literals;  // NOLINT

absl::StatusOr<ManyFields> MakeManyFields() {
  return ParseTextProto<ManyFields>(
      R"pb(field_one {
             repeated_field {}
             repeated_field {
               string_field: "inner_inner_string"
               map_field_uint32 { key: 324 value: "map_value_324" }
               map_field_uint32 { key: 543 value: "map_value_543" }
             }
           }
           map_field_int64 { key: -1345 value: "map_value_-1345" }
           nested_map_bool {
             key: false
             value { string_field: "string_false" }
           }
           nested_map_bool {
             key: true
             value { string_field: "string_true" }
           })pb");
}

absl::StatusOr<::proto2::RepeatedPtrField<FieldIndex>> MakeFieldTags() {
  ASSIGN_OR_RETURN(auto ret, ParseTextProto<ChunkedField>(R"pb(
                     field_tag { field: 2 }
                     field_tag { index: 1505 }
                     field_tag { field: 5 }
                     field_tag { map_key { ui32: 123 } }
                   )pb"));
  return ret.field_tag();
}

absl::StatusOr<::proto2::RepeatedPtrField<FieldIndex>>
MakeFieldTagsTooManyIndices() {
  ASSIGN_OR_RETURN(auto ret, ParseTextProto<ChunkedField>(R"pb(
                     field_tag { field: 2 }
                     field_tag { index: 1505 }
                     field_tag { index: 1506 }
                     field_tag { field: 5 }
                     field_tag { map_key { ui32: 123 } }
                   )pb"));
  return ret.field_tag();
}

absl::StatusOr<::proto2::RepeatedPtrField<FieldIndex>>
MakeFieldTagsTooManyMapKeys() {
  ASSIGN_OR_RETURN(auto ret, ParseTextProto<ChunkedField>(R"pb(
                     field_tag { field: 2 }
                     field_tag { index: 1505 }
                     field_tag { field: 5 }
                     field_tag { map_key { ui32: 123 } }
                     field_tag { map_key: { ui32: 124 } }
                   )pb"));
  return ret.field_tag();
}

absl::StatusOr<::proto2::RepeatedPtrField<FieldIndex>>
MakeFieldTagsMisplacedIndex() {
  ASSIGN_OR_RETURN(auto ret, ParseTextProto<ChunkedField>(R"pb(
                     field_tag { field: 2 }
                     field_tag { index: 1505 }
                     field_tag { field: 5 }
                     field_tag { map_key { ui32: 123 } }
                     field_tag { index: 1504 }
                   )pb"));
  return ret.field_tag();
}

absl::StatusOr<::proto2::RepeatedPtrField<FieldIndex>>
MakeFieldTagsMisplacedMapKey() {
  ASSIGN_OR_RETURN(auto ret, ParseTextProto<ChunkedField>(R"pb(
                     field_tag { field: 2 }
                     field_tag { index: 1505 }
                     field_tag { map_key: { ui32: 321 } }
                     field_tag { field: 5 }
                     field_tag { map_key { ui32: 123 } }
                   )pb"));
  return ret.field_tag();
}

TEST(UtilTest, TestFieldTag) {
  ManyFields message;
  ChunkedField field;
  std::vector<FieldType> fields = {"nested_map_bool"s, true, 2, 50, 7, false};
  ASSERT_OK(AddFieldTag(*message.descriptor(), fields, field));

  EXPECT_THAT(field,
              EqualsProto(R"pb(field_tag { field: 7 }
                               field_tag { map_key { boolean: true } }
                               field_tag { field: 2 }
                               field_tag { index: 50 }
                               field_tag { field: 7 }
                               field_tag { map_key { boolean: false } })pb"));
}
TEST(UtilTest, TestFieldTagWithInt64MapKey) {
  ManyFields message;
  ChunkedField field;
  std::vector<FieldType> fields = {"map_field_int64"s, -4234};
  ASSERT_OK(AddFieldTag(*message.descriptor(), fields, field));

  EXPECT_THAT(field, EqualsProto(R"pb(field_tag { field: 6 }
                                      field_tag { map_key { i64: -4234 } }
              )pb"));
}

TEST(UtilTest, TestFieldTagWithUInt32MapKey) {
  ManyFields message;
  ChunkedField field;
  std::vector<FieldType> fields = {"repeated_field"s, 1505, "map_field_uint32"s,
                                   123};
  ASSERT_OK(AddFieldTag(*message.descriptor(), fields, field));

  EXPECT_THAT(field, EqualsProto(R"pb(field_tag { field: 2 }
                                      field_tag { index: 1505 }
                                      field_tag { field: 5 }
                                      field_tag { map_key { ui32: 123 } }
              )pb"));
}

TEST(UtilTest, TestFieldTagConversion) {
  ManyFields message;
  ChunkedField field;
  std::vector<FieldType> fields = {"repeated_field"s, "1505"s,
                                   "map_field_uint32"s, 123};
  ASSERT_OK(AddFieldTag(*message.descriptor(), fields, field));

  EXPECT_THAT(field, EqualsProto(R"pb(field_tag { field: 2 }
                                      field_tag { index: 1505 }
                                      field_tag { field: 5 }
                                      field_tag { map_key { ui32: 123 } }
              )pb"));
}

TEST(UtilTest, TestGetFieldTypes) {
  ASSERT_OK_AND_ASSIGN(auto tags, MakeFieldTags());
  EXPECT_THAT(GetFieldTypes(tags),
              IsOkAndHolds(std::vector<Field>{{2, 1505}, {5, 123}}));
}

TEST(UtilTest, TestGetFieldTypesThenAddFieldTags) {
  ASSERT_OK_AND_ASSIGN(auto message, MakeManyFields());
  ChunkedField chunked_field;

  ASSERT_OK_AND_ASSIGN(auto tags, MakeFieldTags());
  ASSERT_OK_AND_ASSIGN(std::vector<Field> fields, GetFieldTypes(tags));
  for (const auto& field : fields) {
    ASSERT_OK(AddFieldTag(*message.descriptor(), field, chunked_field));
  }

  EXPECT_THAT(chunked_field,
              EqualsProto(R"pb(field_tag { field: 2 }
                               field_tag { index: 1505 }
                               field_tag { field: 5 }
                               field_tag { map_key { ui32: 123 } }
              )pb"));
}

TEST(UtilTest, TestGetFieldTypesThenAddFieldTagsTooManyIndices) {
  ASSERT_OK_AND_ASSIGN(auto message, MakeManyFields());
  ChunkedField chunked_field;

  ASSERT_OK_AND_ASSIGN(auto tags, MakeFieldTagsTooManyIndices());
  EXPECT_THAT(GetFieldTypes(tags),
              StatusIs(absl::StatusCode::kFailedPrecondition,
                       HasSubstr("Index doesn't belong to any field")));
}

TEST(UtilTest, TestGetFieldTypesThenAddFieldTagsTooManyMapKeys) {
  ASSERT_OK_AND_ASSIGN(auto message, MakeManyFields());
  ChunkedField chunked_field;

  ASSERT_OK_AND_ASSIGN(auto tags, MakeFieldTagsTooManyMapKeys());
  EXPECT_THAT(GetFieldTypes(tags),
              StatusIs(absl::StatusCode::kFailedPrecondition,
                       HasSubstr("Map key doesn't belong to any field")));
}

TEST(UtilTest, TestGetFieldTypesThenAddFieldTagsMisplacedIndex) {
  ASSERT_OK_AND_ASSIGN(auto message, MakeManyFields());
  ChunkedField chunked_field;

  ASSERT_OK_AND_ASSIGN(auto tags, MakeFieldTagsMisplacedIndex());
  EXPECT_THAT(GetFieldTypes(tags),
              StatusIs(absl::StatusCode::kFailedPrecondition,
                       HasSubstr("Index doesn't belong to any field")));
}

TEST(UtilTest, TestGetFieldTypesThenAddFieldTagsMisplacedMapKey) {
  ASSERT_OK_AND_ASSIGN(auto message, MakeManyFields());
  ChunkedField chunked_field;

  ASSERT_OK_AND_ASSIGN(auto tags, MakeFieldTagsMisplacedMapKey());
  EXPECT_THAT(GetFieldTypes(tags),
              StatusIs(absl::StatusCode::kFailedPrecondition,
                       HasSubstr("Map key doesn't belong to any field")));
}

TEST(UtilTest, TestSetRepeatedFieldElement) {
  ASSERT_OK_AND_ASSIGN(auto message, ParseTextProto<ManyFields>(
                                         R"pb(repeated_string_field: ""
                                              repeated_string_field: "")pb"));
  const ::proto2::FieldDescriptor* field_desc =
      message.GetDescriptor()->FindFieldByName("repeated_string_field");
  const std::vector<std::string> chunks = {"repeated_string_one",
                                           "repeated_string_two"};
  ASSERT_OK(SetRepeatedFieldElement(
      &message, field_desc, 0, chunks[0],
      []() -> absl::Status { return absl::OkStatus(); }));
  ASSERT_OK(SetRepeatedFieldElement(
      &message, field_desc, 1, chunks[1],
      []() -> absl::Status { return absl::OkStatus(); }));

  EXPECT_FALSE(message.repeated_string_field().empty());
  EXPECT_EQ(message.repeated_string_field().at(0), "repeated_string_one");
  EXPECT_EQ(message.repeated_string_field().at(1), "repeated_string_two");
}

TEST(UtilTest, TestSetRepeatedFieldElementInvalidIndex) {
  ASSERT_OK_AND_ASSIGN(auto message, ParseTextProto<ManyFields>(
                                         R"pb(repeated_string_field: "")pb"));
  const ::proto2::FieldDescriptor* field_desc =
      message.GetDescriptor()->FindFieldByName("repeated_string_field");

  EXPECT_THAT(SetRepeatedFieldElement(
                  &message, field_desc, 1, "",
                  []() -> absl::Status { return absl::OkStatus(); }),
              StatusIs(absl::StatusCode::kOutOfRange,
                       HasSubstr("Field index out of range")));
}

TEST(UtilTest, TestSetRepeatedFieldElementAlreadyExists) {
  ASSERT_OK_AND_ASSIGN(auto message,
                       ParseTextProto<ManyFields>(
                           R"pb(repeated_string_field: "existing_string")pb"));
  const ::proto2::FieldDescriptor* field_desc =
      message.GetDescriptor()->FindFieldByName("repeated_string_field");
  const std::string chunk = "inner_inner_string_v2";

  EXPECT_OK(SetRepeatedFieldElement(
      &message, field_desc, 0, chunk,
      []() -> absl::Status { return absl::OkStatus(); }));

  EXPECT_FALSE(message.repeated_string_field().empty());
  EXPECT_NE(message.repeated_string_field().at(0), "existing_string");
  EXPECT_EQ(message.repeated_string_field().at(0), "inner_inner_string_v2");
}

TEST(UtilTest, TestSetRepeatedFieldElementBadFieldDesc) {
  ASSERT_OK_AND_ASSIGN(auto message,
                       ParseTextProto<ManyFields>(
                           R"pb(repeated_string_field: "existing_string")pb"));

  const ::proto2::FieldDescriptor* singular_field_desc =
      message.GetDescriptor()->FindFieldByName("string_field");
  EXPECT_DEATH(
      auto status = SetRepeatedFieldElement(
          &message, singular_field_desc, 0, "",
          []() -> absl::Status { return absl::OkStatus(); }),
      HasSubstr("Field is singular; the method requires a repeated field"));

  const ::proto2::FieldDescriptor* map_field_desc =
      message.GetDescriptor()->FindFieldByName("map_field_uint32");
  EXPECT_THAT(SetRepeatedFieldElement(
                  &message, map_field_desc, 0, "",
                  []() -> absl::Status { return absl::OkStatus(); }),
              StatusIs(absl::StatusCode::kFailedPrecondition,
                       HasSubstr("Field is a map")));
}

TEST(UtilTest, TestSetRepeatedFieldElementMessage) {
  ASSERT_OK_AND_ASSIGN(auto message,
                       ParseTextProto<ManyFields>(R"pb(repeated_field {})pb"));
  const ::proto2::FieldDescriptor* field_desc =
      message.GetDescriptor()->FindFieldByName("repeated_field");

  ASSERT_OK_AND_ASSIGN(auto chunk_proto,
                       ParseTextProto<ManyFields>(R"pb(string_field: "")pb"));

  const std::string chunk = chunk_proto.SerializeAsString();

  ASSERT_OK(SetRepeatedFieldElement(
      &message, field_desc, 0, chunk,
      [&message, &field_desc]() -> absl::Status {
        ::proto2::Message* inner_message =
            message.GetReflection()->MutableRepeatedMessage(&message,
                                                            field_desc, 0);
        inner_message->GetReflection()->SetString(
            inner_message,
            field_desc->message_type()->FindFieldByName("string_field"),
            "inner_string");
        return absl::OkStatus();
      }));

  EXPECT_FALSE(message.repeated_field().empty());
  EXPECT_EQ(message.repeated_field().at(0).string_field(), "inner_string");
}

TEST(UtilTest, TestSetFieldElement) {
  ASSERT_OK_AND_ASSIGN(auto message,
                       ParseTextProto<ManyFields>(R"pb(string_field: "")pb"));
  const ::proto2::FieldDescriptor* field_desc =
      message.GetDescriptor()->FindFieldByName("string_field");
  const std::string chunk = "string_field_v2";
  ASSERT_OK(SetFieldElement(&message, field_desc, chunk,
                            []() -> absl::Status { return absl::OkStatus(); }));

  EXPECT_EQ(message.string_field(), "string_field_v2");
}

TEST(UtilTest, TestSetFieldElementInvalidRepeated) {
  ASSERT_OK_AND_ASSIGN(auto message, ParseTextProto<ManyFields>(
                                         R"pb(repeated_string_field: "")pb"));
  const ::proto2::FieldDescriptor* field_desc =
      message.GetDescriptor()->FindFieldByName("repeated_string_field");

  EXPECT_DEATH(
      auto status =
          SetFieldElement(&message, field_desc, "",
                          []() -> absl::Status { return absl::OkStatus(); }),
      HasSubstr("Field is repeated; the method requires a singular field"));
}

TEST(UtilTest, TestSetFieldElementAlreadyExists) {
  ASSERT_OK_AND_ASSIGN(
      auto message,
      ParseTextProto<ManyFields>(R"pb(string_field: "existing_string")pb"));
  const ::proto2::FieldDescriptor* field_desc =
      message.GetDescriptor()->FindFieldByName("string_field");
  const std::string chunk = "inner_inner_string_v2";

  EXPECT_OK(SetFieldElement(&message, field_desc, chunk,
                            []() -> absl::Status { return absl::OkStatus(); }));

  EXPECT_EQ(message.string_field(), "inner_inner_string_v2");
}

TEST(UtilTest, TestSetFieldElementMessage) {
  ASSERT_OK_AND_ASSIGN(auto message,
                       ParseTextProto<ManyFields>(R"pb(field_one {})pb"));
  const ::proto2::FieldDescriptor* field_desc =
      message.GetDescriptor()->FindFieldByName("field_one");

  ASSERT_OK_AND_ASSIGN(auto chunk_proto,
                       ParseTextProto<ManyFields>(R"pb(string_field: "")pb"));

  const std::string chunk = chunk_proto.SerializeAsString();

  ASSERT_OK(SetFieldElement(
      &message, field_desc, chunk, [&message, &field_desc]() -> absl::Status {
        ::proto2::Message* inner_message =
            message.GetReflection()->MutableMessage(&message, field_desc);
        inner_message->GetReflection()->SetString(
            inner_message,
            field_desc->message_type()->FindFieldByName("string_field"),
            "inner_string");
        return absl::OkStatus();
      }));

  EXPECT_TRUE(message.has_field_one());
  EXPECT_EQ(message.field_one().string_field(), "inner_string");
}

TEST(UtilTest, TestAddMapEntry) {
  ASSERT_OK_AND_ASSIGN(auto message, MakeManyFields());
  const ::proto2::FieldDescriptor* field_desc =
      message.GetDescriptor()->FindFieldByName("map_field_int64");
  FieldType map_key = -4234;
  ASSERT_OK(AddMapEntry(&message, field_desc, map_key));

  ASSERT_OK_AND_ASSIGN(int map_entry_index,
                       FindMapKey(message, *field_desc, nullptr, map_key));
  ::proto2::Message* map_entry =
      message.GetReflection()->MutableRepeatedMessage(&message, field_desc,
                                                      map_entry_index);
  map_entry->GetReflection()->SetString(
      map_entry, field_desc->message_type()->FindFieldByNumber(2),
      "map_value_-4234");

  EXPECT_EQ(message.map_field_int64().at(-4234), "map_value_-4234");
}

TEST(UtilTest, GetFieldInvalidIndex) {
  std::vector<FieldType> fields = {"field_one"s, "repeated_field"s, 100};

  ASSERT_OK_AND_ASSIGN(auto message, MakeManyFields());
  EXPECT_THAT(GetField(message, fields),
              StatusIs(absl::StatusCode::kNotFound,
                       HasSubstr("Can't access index 100")));
}

TEST(UtilTest, GetFieldInvalidField) {
  std::vector<FieldType> fields = {"field_one"s, "INVALID"s};
  ASSERT_OK_AND_ASSIGN(auto message, MakeManyFields());
  EXPECT_THAT(GetField(message, fields),
              StatusIs(absl::StatusCode::kNotFound,
                       HasSubstr("Field not found: INVALID")));
}

TEST(UtilTest, GetFieldInvalidMapKey) {
  std::vector<FieldType> fields = {"map_field_int64"s, 10000};
  ASSERT_OK_AND_ASSIGN(auto message, MakeManyFields());
  EXPECT_THAT(GetField(message, fields),
              StatusIs(absl::StatusCode::kNotFound,
                       HasSubstr("couldn't find key: 10000")));
}
TEST(UtilTest, GetField) {
  ASSERT_OK_AND_ASSIGN(auto message, MakeManyFields());
  std::vector<FieldType> fields = {"field_one"s, "repeated_field"s};

  ASSERT_OK_AND_ASSIGN(auto ret, GetField(message, fields));
  EXPECT_EQ(-1, ret.index);
  EXPECT_EQ(2, ret.parent->GetReflection()->FieldSize(*ret.parent, ret.field));

  fields = {"nested_map_bool"s, "true"s, "string_field"s};
  ASSERT_OK_AND_ASSIGN(auto ret2, GetField(message, fields));
  EXPECT_EQ(-1, ret2.index);
  EXPECT_EQ("string_true",
            ret2.parent->GetReflection()->GetString(*ret2.parent, ret2.field));
}

TEST(UtilTest, FindMapKey) {
  ASSERT_OK_AND_ASSIGN(auto message, MakeManyFields());
  const ::proto2::FieldDescriptor* map_field =
      message.GetDescriptor()->FindFieldByName("nested_map_bool");
  ASSERT_OK_AND_ASSIGN(int i, FindMapKey(message, *map_field, nullptr, true));
  EXPECT_EQ(1, i);
  ASSERT_OK_AND_ASSIGN(i, FindMapKey(message, *map_field, nullptr, false));
  EXPECT_EQ(0, i);

  map_field = message.GetDescriptor()->FindFieldByName("map_field_int64");
  ASSERT_OK_AND_ASSIGN(i, FindMapKey(message, *map_field, nullptr, -1345));
  EXPECT_EQ(0, i);
  ASSERT_OK_AND_ASSIGN(i, FindMapKey(message, *map_field, nullptr, 1345));
  EXPECT_EQ(-1, i);
}

TEST(UtilTest, FindMapKeyInvalid) {
  ASSERT_OK_AND_ASSIGN(auto message, MakeManyFields());
  const ::proto2::FieldDescriptor* not_map_field =
      message.GetDescriptor()->FindFieldByName("string_field");
  EXPECT_THAT(FindMapKey(message, *not_map_field, nullptr, -1345),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("was given a non map field")));
}

TEST(UtilTest, TestHumanReadableBytes) {
  EXPECT_EQ("1.0KiB", HumanReadableBytes(1024));
  EXPECT_EQ("5.5KiB", HumanReadableBytes(5632));
  EXPECT_EQ("52.2KiB", HumanReadableBytes(53432));
  EXPECT_EQ("72.9MiB", HumanReadableBytes(76493281));
  EXPECT_EQ("57.0MiB", HumanReadableBytes(5.977e7));
  EXPECT_EQ("1.0GiB", HumanReadableBytes(1.074e9));
  EXPECT_EQ("15.4GiB", HumanReadableBytes(16493342281));
}

TEST(UtilTest, TestHumanReadableDuration) {
  EXPECT_EQ("534 microseconds", HumanReadableDuration(534));
  EXPECT_EQ("1.00 ms", HumanReadableDuration(1000));
  EXPECT_EQ("14.33 ms", HumanReadableDuration(14328));
  EXPECT_EQ("95.83 s", HumanReadableDuration(95825433));
}

TEST(UtilTest, TestReadChunk) {
  std::string cpb_file = file::JoinPath(::proto_splitter::SrcRoot(), "testdata",
                                        "df-split-tree.cpb");

  ASSERT_OK_AND_ASSIGN(auto reader, GetRiegeliReader(cpb_file));

  auto read_metadata = GetChunkMetadata(reader);
  if (!read_metadata.ok()) {
    reader.Close();
    ASSERT_OK(read_metadata.status());
  }
  ChunkMetadata metadata = read_metadata.value();
  std::vector<ChunkInfo> chunks_info(metadata.chunks().begin(),
                                     metadata.chunks().end());

  for (const auto& chunk_info : chunks_info) {
    ASSERT_OK_AND_ASSIGN(std::string chunk, ReadChunk(reader, chunk_info));
    ASSERT_EQ(chunk.size(), chunk_info.size());
  }
}

}  // namespace
}  // namespace proto_splitter
