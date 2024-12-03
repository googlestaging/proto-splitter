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
#ifndef PROTO_SPLITTER_CC_TEST_UTIL_H_
#define PROTO_SPLITTER_CC_TEST_UTIL_H_

#include <cstdlib>
#include <ostream>
#include <string>

#include "file/base/path.h"
#include "testing/base/public/gmock.h"
#include "third_party/absl/log/check.h"
#include "third_party/absl/log/log.h"
#include "third_party/absl/status/status.h"
#include "third_party/proto_splitter/platform.h"
#include "third_party/protobuf/io/zero_copy_stream_impl_lite.h"
#include "third_party/protobuf/message.h"
#include "third_party/protobuf/text_format.h"

namespace proto_splitter {

// Constant which is false internally and true in open source.
inline constexpr bool kIsOpenSource = PROTO_SPLITTER_IS_IN_OSS;

std::string GetEnvVarOrDie(const char* env_var) {
  const char* value = std::getenv(env_var);
  if (!value) {
    LOG(FATAL) << "Failed to find environment variable:" << env_var;
  }
  return value;
}

std::string SrcRoot() {
  std::string workspace = GetEnvVarOrDie("TEST_WORKSPACE");
  std::string srcdir = GetEnvVarOrDie("TEST_SRCDIR");

  return kIsOpenSource
             ? file::JoinPath(srcdir, workspace, "proto_splitter")
             : file::JoinPath(srcdir, workspace, "third_party/proto_splitter");
}

inline std::string SerializeAsString(const ::proto2::Message& message) {
  std::string result;
  {
    // Use a nested block to guarantee triggering coded_stream's destructor
    // before `result` is used. Due to copy elision, this code works without
    // the nested block, but small, innocent looking changes can break it.
    ::proto2::io::StringOutputStream string_stream(&result);
    ::proto2::io::CodedOutputStream coded_stream(&string_stream);
    coded_stream.SetSerializationDeterministic(true);
    message.SerializeToCodedStream(&coded_stream);
  }
  return result;
}

template <typename MessageType>
absl::StatusOr<MessageType> ParseTextProto(const char* text_proto) {
  ::proto2::TextFormat::Parser parser;
  MessageType parsed_proto;
  bool success =
      ::proto2::TextFormat::ParseFromString(text_proto, &parsed_proto);
  if (!success) {
    return absl::InvalidArgumentError(
        "Input text could not be parsed to proto.");
  }
  return parsed_proto;
}

// TODO(b/229726259): EqualsProto is not available in OSS.

// Simple implementation of a proto matcher comparing string representations.
// This will fail if the string representations are not deterministic.
class ProtoStringMatcher {
 public:
  explicit ProtoStringMatcher(const ::proto2::Message& expected)
      : expected_(SerializeAsString(expected)) {}

  explicit ProtoStringMatcher(const std::string& expected)
      : text_format_(expected), use_text_format_(true) {}

  template <typename Message>
  bool MatchAndExplain(const Message& p,
                       ::testing::MatchResultListener*) const {
    if (use_text_format_) {
      Message* m = p.New();
      CHECK(::proto2::TextFormat::ParseFromString(text_format_, m))
          << "Failed to parse proto text: " << text_format_;
      std::string expected = SerializeAsString(*m);
      delete m;
      return SerializeAsString(p) == expected;
    } else {
      return SerializeAsString(p) == expected_;
    }
  }

  void DescribeTo(::std::ostream* os) const {
    if (use_text_format_)
      *os << text_format_;
    else
      *os << expected_;
  }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "not equal to expected message: ";
    auto out_message = use_text_format_ ? &text_format_ : &expected_;
    if (out_message->size() < 1e5)
      *os << *out_message;
    else
      *os << "(too large to print) \n";
  }

 private:
  const std::string expected_;
  const std::string text_format_;
  const bool use_text_format_ = false;
};

inline ::testing::PolymorphicMatcher<ProtoStringMatcher> EqualsProto(
    const ::proto2::Message& x) {
  return ::testing::MakePolymorphicMatcher(ProtoStringMatcher(x));
}

inline ::testing::PolymorphicMatcher<ProtoStringMatcher> EqualsProto(
    const std::string& x) {
  return ::testing::MakePolymorphicMatcher(ProtoStringMatcher(x));
}

}  // namespace proto_splitter

#endif  // PROTO_SPLITTER_CC_TEST_UTIL_H_
