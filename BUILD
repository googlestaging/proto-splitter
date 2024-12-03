# Description:
#   Utilities for splitting and joining large protos >2GB.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_proto_library", "cc_test")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_google_protobuf//:protobuf.bzl", "py_proto_library")
load("@rules_python//python:defs.bzl", "py_library", "py_test")
load("@rules_license//rules:license.bzl", "license")

package(
    default_applicable_licenses = [":license"],
    default_visibility = ["//visibility:public"],
)

license(
    name = "license",
    package_name = "proto_splitter",
)

licenses(["notice"])

exports_files(["LICENSE"])

proto_library(
    name = "versions_proto",
    srcs = ["versions.proto"],
)

cc_proto_library(
    name = "versions_cc_proto",
    deps = [":versions_proto"],
)

py_proto_library(
    name = "versions_py_proto",
    deps = [
        ":versions_proto",
    ],
)

proto_library(
    name = "chunk_proto",
    srcs = ["chunk.proto"],
    deps = [":versions_proto"],
)

cc_proto_library(
    name = "chunk_cc_proto",
    deps = [":chunk_proto"],
)

py_proto_library(
    name = "chunk_py_proto",
    deps = [
        ":chunk_proto",
    ],
)

py_library(
    name = "split",
    srcs = ["split.py"],
    deps = [
        ":chunk_py_proto",
        ":util",
        ":version",
        ":versions_py_proto",
        "@absl_py//absl/logging",
        "@etils//etils/epath",
        "@riegeli_py//python/riegeli",
    ],
)

py_library(
    name = "version",
    srcs = ["version.py"],
)

py_test(
    name = "split_test",
    srcs = ["split_test.py"],
    tags = [
        "no_mac",  # b/291933687
        "no_windows",  # b/291001524
    ],
    deps = [
        ":chunk_py_proto",
        ":split",
        "//testdata:test_message_py_proto",
        "@absl_py//absl/testing:absl_test",
        #internal proto upb dep
        "@riegeli_py//python/riegeli",
    ],
)

py_library(
    name = "constants",
    srcs = ["constants.py"],
    visibility = ["//visibility:public"],
)

py_library(
    name = "util",
    srcs = ["util.py"],
    deps = [
        ":chunk_py_proto",
    ],
)

py_test(
    name = "util_test",
    srcs = ["util_test.py"],
    deps = [
        ":util",
        "//testdata:test_message_py_proto",
        "@absl_py//absl/testing:absl_test",
        #internal proto upb dep
    ],
)

cc_library(
    name = "merge_impl",
    srcs = [
        "merge.cc",
        "merge.h",
    ],
    deps = [
        ":chunk_cc_proto",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "//cc:util",
        "@com_google_protobuf//:protobuf",
        "@riegeli_cc//riegeli/base:object",
        "@riegeli_cc//riegeli/bytes:fd_reader",
        "@riegeli_cc//riegeli/records:record_reader",
    ],
)

cc_library(
    name = "merge",
    hdrs = ["merge.h"],
    deps = [
        ":chunk_cc_proto",
        ":merge_impl",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:string_view",
        "@com_google_protobuf//:protobuf",
        "@riegeli_cc//riegeli/bytes:fd_reader",
        "@riegeli_cc//riegeli/records:record_reader",
    ],
)

cc_test(
    name = "merge_test",
    srcs = ["merge_test.cc"],
    data = [
        "//testdata:bf-split-tree.cpb",
        "//testdata:bf-split-tree.pbtxt",
        "//testdata:df-split-tree.cpb",
        "//testdata:df-split-tree.pbtxt",
        "//testdata:many-field.cpb",
        "//testdata:many-field.pbtxt",
        "//testdata:split-tree.pbtxt",
    ],
    tags = [
        "no_mac",  # b/291933687
        "no_windows",  # b/291001524
    ],
    deps = [
        ":chunk_cc_proto",
        ":merge",
        "@googletest//:gtest_main",
        "@com_google_absl//absl/strings",
        "//cc:test_util",
        "//cc:util",
        "//testdata:test_message_cc_proto",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//protobuf:io",
    ],
)

cc_library(
    name = "platform",
    hdrs = ["platform.h"],
    visibility = [
        "__subpackages__",
    ],
)
