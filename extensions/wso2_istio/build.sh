
bazel build istio_example.wasm
rm -rf istio_example.wasm
cp ../../bazel-bin/extensions/wso2_istio/istio_example.wasm .

date_value=$(date)
git commit -a -m "$date_value"
git push origin master

sha256sum istio_example.wasm