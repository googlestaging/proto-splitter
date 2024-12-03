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

#ifndef PROTO_SPLITTER_CC_SPLIT_H_
#define PROTO_SPLITTER_CC_SPLIT_H_

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "third_party/absl/status/status.h"
#include "third_party/absl/status/statusor.h"
#include "third_party/proto_splitter/chunk.proto.h"
#include "third_party/proto_splitter/versions.proto.h"
#include "third_party/protobuf/message.h"

namespace proto_splitter {

using ::proto_splitter::ChunkedMessage;
using ::proto_splitter::VersionDef;
using MessageBytes = std::variant<std::shared_ptr<::proto2::Message>,
                                  ::proto2::Message*, std::string>;

// Interface for proto message splitters.
class Splitter {
 public:
  virtual ~Splitter() = default;

  // Split message into chunks.
  virtual absl::StatusOr<std::pair<std::vector<MessageBytes>*, ChunkedMessage*>>
  Split() = 0;

  // Write message to disk.
  virtual absl::Status Write(std::string file_prefix) = 0;

  // Version info about the Splitter and required Merger versions.
  virtual VersionDef Version() = 0;
};

}  // namespace proto_splitter

#endif  // PROTO_SPLITTER_CC_SPLIT_H_
