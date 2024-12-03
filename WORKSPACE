workspace(name = "proto_splitter")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Abseil
http_archive(
    name = "com_google_absl",
    urls = ["https://github.com/abseil/abseil-cpp/releases/download/20240722.0/abseil-cpp-20240722.0.tar.gz"],
    strip_prefix = "abseil-cpp-20240722.0",
)

http_archive(
    name = "absl_py",
    sha256 = "a7c51b2a0aa6357a9cbb2d9437e8cd787200531867dc02565218930b6a32166e",
    urls = ["https://github.com/abseil/abseil-py/archive/refs/tags/v1.0.0.tar.gz"],
    strip_prefix = "abseil-py-1.0.0",
)

# Protobuf
http_archive(
    name = "com_google_protobuf",
    sha256 = "f66073dee0bc159157b0bd7f502d7d1ee0bc76b3c1eac9836927511bdc4b3fc1",
    urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.21.9.zip"],
    strip_prefix = "protobuf-3.21.9",
)
load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")
protobuf_deps()

# Google Test
http_archive(
    name = "googletest",
    sha256 = "81964fe578e9bd7c94dfdb09c8e4d6e6759e19967e397dbea48d1c10e45d0df2",
    urls = ["https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz"],
    strip_prefix = "googletest-release-1.12.1",
)

# Riegeli is imported twice since there are two targets (third_party/riegeli and
# third_party/py/riegeli) that are used in the Proto Splitter.
http_archive(
    name = "riegeli_cc",
    sha256 = "1d216d5c97fa60632143d209a1bb48c2a83788efdb876902e7bbc06396d5ee1f",
    urls = ["https://github.com/google/riegeli/archive/5d75119232cd4f6db8dfa69a1503289f050e9643.zip"],
    strip_prefix = "riegeli-5d75119232cd4f6db8dfa69a1503289f050e9643",
)

http_archive(
    name = "riegeli_py",
    sha256 = "1d216d5c97fa60632143d209a1bb48c2a83788efdb876902e7bbc06396d5ee1f",
    urls = ["https://github.com/google/riegeli/archive/5d75119232cd4f6db8dfa69a1503289f050e9643.zip"],
    strip_prefix = "riegeli-5d75119232cd4f6db8dfa69a1503289f050e9643",
)

# Required by riegeli.
http_archive(
    name = "org_brotli",
    sha256 = "84a9a68ada813a59db94d83ea10c54155f1d34399baf377842ff3ab9b3b3256e",
    strip_prefix = "brotli-3914999fcc1fda92e750ef9190aa6db9bf7bdb07",
    urls = ["https://github.com/google/brotli/archive/3914999fcc1fda92e750ef9190aa6db9bf7bdb07.zip"],  # 2022-11-17
)

http_archive(
    name = "net_zstd",
    build_file = "//third_party:net_zstd.BUILD",
    sha256 = "b6c537b53356a3af3ca3e621457751fa9a6ba96daf3aebb3526ae0f610863532",
    strip_prefix = "zstd-1.4.5/lib",
    urls = ["https://github.com/facebook/zstd/archive/v1.4.5.zip"],  # 2020-05-22
)

http_archive(
    name = "highwayhash",
    urls = ["https://github.com/google/highwayhash/archive/c13d28517a4db259d738ea4886b1f00352a3cc33.tar.gz"],
    sha256 = "c0e2b9931fbcce3bfbcd7999c3c114f404ac0f8b89775a5bbccbcaa501868e58",
    strip_prefix = "highwayhash-c13d28517a4db259d738ea4886b1f00352a3cc33",
    build_file = "//third_party/highwayhash:highwayhash.BUILD",
)

http_archive(
    name = "snappy",
    build_file = "//third_party:snappy.BUILD",
    sha256 = "7ee7540b23ae04df961af24309a55484e7016106e979f83323536a1322cedf1b",
    strip_prefix = "snappy-1.2.0",
    urls = ["https://github.com/google/snappy/archive/1.2.0.zip"],
)

# Etils
http_archive(
    name = "etils",
    urls = ["https://github.com/google/etils/archive/refs/tags/v1.9.4.tar.gz"],
    strip_prefix = "etils-1.9.4",
)

# Bazel Features
http_archive(
    name = "bazel_features",
    sha256 = "ba1282c1aa1d1fffdcf994ab32131d7c7551a9bc960fbf05f42d55a1b930cbfb",
    strip_prefix = "bazel_features-1.15.0",
    url = "https://github.com/bazel-contrib/bazel_features/releases/download/v1.15.0/bazel_features-v1.15.0.tar.gz",
)
load("@bazel_features//:deps.bzl", "bazel_features_deps")
bazel_features_deps()

# Proto Rules
http_archive(
    name = "rules_proto",
    sha256 = "6fb6767d1bef535310547e03247f7518b03487740c11b6c6adb7952033fe1295",
    urls = ["https://github.com/bazelbuild/rules_proto/archive/refs/tags/6.0.2.tar.gz"],
    strip_prefix = "rules_proto-6.0.2",
)
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies")
rules_proto_dependencies()

# CC Rules
http_archive(
    name = "rules_cc",
    sha256 = "d75a040c32954da0d308d3f2ea2ba735490f49b3a7aa3e4b40259ca4b814f825",
    urls = ["https://github.com/bazelbuild/rules_cc/releases/download/0.0.10-rc1/rules_cc-0.0.10-rc1.tar.gz"],
    strip_prefix = "rules_cc-0.0.10-rc1",
)
load("@rules_cc//cc:repositories.bzl", "rules_cc_dependencies")
rules_cc_dependencies()

# Python Rules
http_archive(
    name = "rules_python",
    sha256 = "ca77768989a7f311186a29747e3e95c936a41dffac779aff6b443db22290d913",
    url = "https://github.com/bazelbuild/rules_python/releases/download/0.36.0/rules_python-0.36.0.tar.gz",
    strip_prefix = "rules_python-0.36.0",
)
load("@rules_python//python:pip.bzl", "pip_parse")
pip_parse(
    name = "pip_deps",
    requirements_lock = "//:pip_requirements.txt",
    python_interpreter = "/opt/homebrew/bin/python3.11",
)
load("@pip_deps//:requirements.bzl", "install_deps")
install_deps()

# License Rules
http_archive(
    name = "rules_license",
    sha256 = "241b06f3097fd186ff468832150d6cc142247dc42a32aaefb56d0099895fd229",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_license/releases/download/0.0.8/rules_license-0.0.8.tar.gz",
        "https://github.com/bazelbuild/rules_license/releases/download/0.0.8/rules_license-0.0.8.tar.gz",
    ],
)

# Hedron's Compile Commands Extractor for Bazel
# https://github.com/hedronvision/bazel-compile-commands-extractor
http_archive(
    name = "hedron_compile_commands",

    # Replace the commit hash (0e990032f3c5a866e72615cf67e5ce22186dcb97) in both places (below) with the latest (https://github.com/hedronvision/bazel-compile-commands-extractor/commits/main), rather than using the stale one here.
    # Even better, set up Renovate and let it do the work for you (see "Suggestion: Updates" in the README).
    url = "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/0e990032f3c5a866e72615cf67e5ce22186dcb97.tar.gz",
    strip_prefix = "bazel-compile-commands-extractor-0e990032f3c5a866e72615cf67e5ce22186dcb97",
    # When you first run this tool, it'll recommend a sha256 hash to put here with a message like: "DEBUG: Rule 'hedron_compile_commands' indicated that a canonical reproducible form can be obtained by modifying arguments sha256 = ..."
)
load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")
hedron_compile_commands_setup()
load("@hedron_compile_commands//:workspace_setup_transitive.bzl", "hedron_compile_commands_setup_transitive")
hedron_compile_commands_setup_transitive()
load("@hedron_compile_commands//:workspace_setup_transitive_transitive.bzl", "hedron_compile_commands_setup_transitive_transitive")
hedron_compile_commands_setup_transitive_transitive()
load("@hedron_compile_commands//:workspace_setup_transitive_transitive_transitive.bzl", "hedron_compile_commands_setup_transitive_transitive_transitive")
hedron_compile_commands_setup_transitive_transitive_transitive()
