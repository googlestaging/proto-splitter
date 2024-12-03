# Description:
#   Utilities for splitting and joining large protos > 2GB.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(
    default_applicable_licenses = ["//:license"],  # copybara:comment
    default_visibility = [
        "//learning/brain/tfrt/aot:__subpackages__",
        "//learning/brain/tfrt/tf_tpu:__subpackages__",
        "//:__subpackages__",
        "//third_party/tensorflow/cc/saved_model:__subpackages__",
        "//third_party/tensorflow/cc/saved_model/image_format:__subpackages__",
    ],
    licenses = ["notice"],
)

cc_library(
    name = "split",
    hdrs = ["split.h"],
    deps = [
        ":util",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "//:chunk_cc_proto",
        "//:versions_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "composable_splitter_base",
    srcs = ["composable_splitter_base.cc"],
    hdrs = ["composable_splitter_base.h"],
    deps = [
        ":max_size",
        ":split",
        ":util",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/time",
        "//:chunk_cc_proto",
        "//:versions_cc_proto",
        "@com_google_protobuf//:protobuf",
        "@riegeli_cc//riegeli/bytes:cord_writer",
        "@riegeli_cc//riegeli/bytes:fd_writer",
        "@riegeli_cc//riegeli/bytes:string_writer",
        "@riegeli_cc//riegeli/messages:message_serialize",
        "@riegeli_cc//riegeli/records:record_writer",
    ],
)

cc_library(
    name = "composable_splitter",
    hdrs = ["composable_splitter.h"],
    deps = [
        ":composable_splitter_base",
        ":util",
        "//:chunk_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "composable_splitter_test",
    srcs = ["composable_splitter_test.cc"],
    data = [],
    tags = [
        "no_mac",  # b/291933687
        "no_windows",  # b/291001524
    ],
    deps = [
        ":composable_splitter",
        ":split",
        ":test_util",
        ":util",
        "@googletest//:gtest_main",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
        "//:chunk_cc_proto",
        "//testdata:test_message_cc_proto",
        "@com_google_protobuf//:protobuf",
        "@riegeli_cc//riegeli/bytes:cord_reader",
        "@riegeli_cc//riegeli/bytes:fd_reader",
        "@riegeli_cc//riegeli/bytes:string_reader",
        "@riegeli_cc//riegeli/records:record_reader",
    ],
)

cc_library(
    name = "util",
    srcs = ["util.cc"],
    hdrs = ["util.h"],
    deps = [
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "//:chunk_cc_proto",
        "@com_google_protobuf//:protobuf",
        "@riegeli_cc//riegeli/base:types",
        "@riegeli_cc//riegeli/bytes:fd_reader",
        "@riegeli_cc//riegeli/records:record_reader",
    ],
)

cc_test(
    name = "util_test",
    srcs = ["util_test.cc"],
    data = [
        "//testdata:df-split-tree.cpb",
    ],
    tags = [
        "no_mac",  # b/291933687
        "no_windows",  # b/291001524
    ],
    deps = [
        ":test_util",
        ":util",
        "@googletest//:gtest_main",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "//:chunk_cc_proto",
        "//testdata:test_message_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "size_splitter",
    hdrs = ["size_splitter.h"],
    deps = [
        ":composable_splitter",
        ":util",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "repeated_field_splitter",
    srcs = ["repeated_field_splitter.cc"],
    hdrs = ["repeated_field_splitter.h"],
    deps = [
        ":composable_splitter",
        ":max_size",
        ":size_splitter",
        ":util",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "repeated_field_splitter_test",
    srcs = ["repeated_field_splitter_test.cc"],
    data = [
        "//testdata:df-split-tree.cpb",
    ],
    tags = [
        "no_mac",  # b/291933687
        "no_windows",  # b/291001524
    ],
    deps = [
        ":composable_splitter",
        ":large_node_splitter",
        ":max_size",
        ":repeated_field_splitter",
        ":size_splitter",
        ":test_util",
        ":util",
        "@googletest//:gtest_main",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "//testdata:test_message_cc_proto",
    ],
)

cc_library(
    name = "max_size",
    srcs = ["max_size.cc"],
    hdrs = ["max_size.h"],
    deps = [
        "@com_google_absl//absl/synchronization"
    ],
)

cc_library(
    name = "large_node_splitter",
    hdrs = ["large_node_splitter.h"],
    deps = [
        ":composable_splitter",
        ":composable_splitter_base",
        ":max_size",
        ":size_splitter",
        ":split",
        ":util",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "large_node_splitter_test",
    srcs = ["large_node_splitter_test.cc"],
    deps = [
        ":large_node_splitter",
        ":max_size",
        ":size_splitter",
        ":test_util",
        ":util",
        "@googletest//:gtest_main",
        "@com_google_absl//absl/status:statusor",
        "//testdata:test_message_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "test_util",
    testonly = True,
    hdrs = ["test_util.h"],
    deps = [
        "@googletest//:gtest",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "//:platform",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//protobuf/io",
    ],
)
