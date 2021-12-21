load("@proxy_wasm_cpp_sdk//bazel/wasm:wasm.bzl", "wasm_cc_binary")

licenses(["notice"])  # Apache 2

wasm_cc_binary(
    name = "istio_example.wasm",
    srcs = ["istio_wasm.cc","istio_wasm.h"],
    deps = [
        "@io_istio_proxy//extensions/common/wasm:json_util",
        "@proxy_wasm_cpp_sdk//:proxy_wasm_intrinsics",
    ],
)

