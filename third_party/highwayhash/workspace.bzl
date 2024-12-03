"""Loads the highwayhash library, used by the Proto Splitter."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def repo():
    http_archive(
        name = "highwayhash",
        urls = ["https://github.com/google/highwayhash/archive/c13d28517a4db259d738ea4886b1f00352a3cc33.tar.gz"],
        sha256 = "c0e2b9931fbcce3bfbcd7999c3c114f404ac0f8b89775a5bbccbcaa501868e58",
        strip_prefix = "highwayhash-c13d28517a4db259d738ea4886b1f00352a3cc33",
        build_file = "//third_party/highwayhash:highwayhash.BUILD",
    )