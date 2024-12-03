load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_google_protobuf//:protobuf.bzl", "py_proto_library")
load("@rules_python//python:defs.bzl", "py_binary")

package(
    default_applicable_licenses = ["//proto_splitter:license"],  # copybara:comment
    default_visibility = ["//proto_splitter:__subpackages__"],
    licenses = ["notice"],
)

exports_files([
    "df-split-tree.cpb",
    "df-split-tree.pbtxt",
    "bf-split-tree.cpb",
    "bf-split-tree.pbtxt",
    "split-tree.pbtxt",
    "many-field.cpb",
    "many-field.pbtxt",
])

proto_library(
    name = "test_message_proto",
    srcs = ["test_message.proto"],
)

# copybara:comment_begin(google-only)

py_proto_library(
    name = "test_message_py_proto",
    deps = [
        ":test_message_proto",
    ],
)

cc_proto_library(
    name = "test_message_cc_proto",
    deps = [":test_message_proto"],
)

py_binary(
    name = "split_gen",
    srcs = ["split_gen.py"],
    deps = [
        ":test_message_py_proto",
        "//proto_splitter:chunk_py_proto",
        "//proto_splitter:split",
        "//proto_splitter:util",
        "@absl_py//absl:app",
        "@absl_py//absl/flags",
        "@absl_py//absl/logging",
        "@etils//etils/epath",
    ],
)

py_binary(
    name = "many_field_gen",
    srcs = ["many_field_gen.py"],
    deps = [
        ":test_message_py_proto",
        "//proto_splitter:split",
        "@absl_py//absl:app",
        "@absl_py//absl/flags",
        "@etils//etils/epath",
    ],
)

# copybara:comment_end
