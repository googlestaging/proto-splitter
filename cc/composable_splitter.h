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
#ifndef PROTO_SPLITTER_CC_COMPOSABLE_SPLITTER_H_
#define PROTO_SPLITTER_CC_COMPOSABLE_SPLITTER_H_

#include <vector>

#include "third_party/proto_splitter/cc/composable_splitter_base.h"
#include "third_party/proto_splitter/cc/util.h"
#include "third_party/proto_splitter/chunk.proto.h"
#include "third_party/protobuf/message.h"

namespace proto_splitter {

// A Splitter that can be composed with other splitters.
class ComposableSplitter : public ComposableSplitterBase {
 public:
  // Initializer.
  explicit ComposableSplitter(::proto2::Message* message)
      : ComposableSplitterBase(message), message_(message) {}

  // Initialize a child ComposableSplitter.
  explicit ComposableSplitter(::proto2::Message* message,
                              ComposableSplitterBase* parent_splitter,
                              std::vector<FieldType>* fields_in_parent)
      : ComposableSplitterBase(message, parent_splitter, fields_in_parent),
        message_(message) {}

 protected:
  ::proto2::Message* message() { return message_; }

 private:
  ::proto2::Message* message_;
};

}  // namespace proto_splitter

#endif  // PROTO_SPLITTER_CC_COMPOSABLE_SPLITTER_H_
