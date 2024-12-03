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
#ifndef PROTO_SPLITTER_CC_UTIL_H_
#define PROTO_SPLITTER_CC_UTIL_H_

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "proto_splitter/chunk.proto.h"
#include "protobuf/descriptor.h"
#include "protobuf/message.h"
#include "riegeli/bytes/file_reader.h"
#include "riegeli/records/record_reader.h"

namespace proto_splitter {

// TODO(b/282796592): Consider switching to `::proto2::FieldPath` in the
// future.

// Fields can be represented using their name (string) or number (int). Map key
// and repeated field index types are also included in this variant type.
using FieldType = std::variant<std::string, int, bool>;
using Field = std::pair<FieldType, std::optional<FieldType>>;

// Convert a sequence of field tags to a vector of fields. A single field is a
// std::vector<FieldType>, since multiple field tags may correspond to a single
// field when the field is repeated or a map.
absl::StatusOr<const std::vector<Field>> GetFieldTypes(
    const ::proto2::RepeatedPtrField<::proto_splitter::FieldIndex>& field_tags);

// Sets message.field_desc[field_index] to the data contained in chunk,
// according to the (cpp) type described by field_desc. Uses message_callback
// (instead of simply assigning) when field_desc describes a message.
absl::Status SetRepeatedFieldElement(
    ::proto2::Message* message, const ::proto2::FieldDescriptor* field_desc,
    uint64_t field_index, const std::string& chunk,
    std::function<absl::Status(void)> message_callback);

// Sets message.field_desc to the data contained in chunk, according to the
// (cpp) type described by field_desc. Uses message_callback (instead of simply
// assigning) when field_desc describes a message.
absl::Status SetFieldElement(
    ::proto2::Message* message, const ::proto2::FieldDescriptor* field_desc,
    const std::string& chunk,
    std::function<absl::Status(void)> message_callback);

// Adds a new map entry (repeated message element with key/value fields) to
// message.field_desc (a map). The new map entry's key is set using map_key,
// according to its type.
absl::Status AddMapEntry(::proto2::Message* message,
                         const ::proto2::FieldDescriptor* field_desc,
                         FieldType map_key);

// Struct returned by `GetMutableField`. The field can be retrieved by using the
// Reflection API on the parent.
struct MutableFieldResult {
  ::proto2::Message* parent;
  const ::proto2::FieldDescriptor* field;
  // If field is repeated or map, `index` is set to the list index or the
  // position at which the map key appears. If the field is not repeated,
  // `index` is -1.
  int index;
};

// Returns a mutable parent, field descriptor, and int index in the case of a
// repeated or map value field (or -1 if a non-repeated/map field).
absl::StatusOr<MutableFieldResult> GetMutableField(
    ::proto2::Message* message, const std::vector<FieldType>& fields);

absl::StatusOr<MutableFieldResult> GetMutableField(::proto2::Message* message,
                                                   const Field& field);

// Gets info about the mutable field that's directly attached to message.
absl::StatusOr<MutableFieldResult> GetMutableField(::proto2::Message* message,
                                                   const FieldType& field_type);

// Struct returned by `GetField`. The field can be retrieved by using the
// Reflection API on the parent.
struct FieldResult {
  const ::proto2::Message* parent;
  const ::proto2::FieldDescriptor* field;
  // If field is repeated or map, `index` is set to the list index or the
  // position at which the map key appears. If the field is not repeated,
  // `index` is -1.
  int index;
};

// Returns the parent message, field descriptor, and int index from following
// the provided message and fields.
absl::StatusOr<FieldResult> GetField(const ::proto2::Message& message,
                                     const std::vector<FieldType>& fields);

// Updates `field_tag` in the ChunkedField proto.
absl::Status AddFieldTag(const ::proto2::Descriptor& desc,
                         const std::vector<FieldType>& fields,
                         ::proto_splitter::ChunkedField& chunked_field);

absl::Status AddFieldTag(const ::proto2::Descriptor& desc, const Field& field,
                         ::proto_splitter::ChunkedField& chunked_field);

// Returns the index of the map key in the map field. If the key is not found,
// returns -1.
absl::StatusOr<int> FindMapKey(const ::proto2::Message& parent,
                               const ::proto2::FieldDescriptor& map_field,
                               const ::proto2::FieldDescriptor* key_field,
                               FieldType map_key);

// Formats bytes into something more readable. (e.g. 52428800 -> "50.0MiB")
std::string HumanReadableBytes(int64_t byte_count);
// Formats microseconds into a more readable string.
std::string HumanReadableDuration(int64_t microseconds);

// Construct a reader object to read in records from the .cpb file.
absl::StatusOr<riegeli::RecordReader<riegeli::FileReader<>>> GetRiegeliReader(
    absl::string_view cpb_file);

// Read the last chunk, which contains metadata necessary for reading the
// remaining chunks.
absl::StatusOr<::proto_splitter::ChunkMetadata> GetChunkMetadata(
    riegeli::RecordReaderBase& reader);

// Use the `reader` to read in the chunk specified by `chunk_info`.
absl::StatusOr<std::string> ReadChunk(
    riegeli::RecordReaderBase& reader,
    const ::proto_splitter::ChunkInfo& chunk_info);

// Returns true if prefix can only be found as a .pb file, and false if a .cpb
// file exists. Returns an error if neither .pb nor .cpb exist.
absl::StatusOr<bool> OnlyContainsPb(absl::string_view prefix);

}  // namespace proto_splitter

#endif  // PROTO_SPLITTER_CC_UTIL_H_
