cc_library(
    name = "oss_sdk_external",
    srcs = glob([
        "sdk/src/external/**/*.cpp",
    ]),
    hdrs = glob([
        "sdk/src/external/**/*.h",
    ]),
    includes = [
        "sdk/src/external",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "oss_sdk",
    srcs = glob([
        "sdk/src/*.cc",
        "sdk/src/**/*.cc",
    ]),
    hdrs = glob([
        "sdk/include/alibabacloud/oss/*.h",
        "sdk/include/alibabacloud/oss/**/*.h",
        "sdk/src/*.h",
        "sdk/src/**/*.h",
    ]),
    includes = [
        "sdk/include",
        "sdk/include/alibabacloud/oss",
        "sdk/src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":oss_sdk_external",
        "@com_github_curl//:curl",
        "@com_github_openssl_openssl//:openssl",
    ],
)
