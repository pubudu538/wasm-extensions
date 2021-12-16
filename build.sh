git pull --rebase pubudu master
cd example/
bazel build //:example.wasm
cp /home/pubududemo/.cache/bazel/_bazel_pubududemo/0801b2896c776c9311f86b69c49ba8f4/execroot/example_extension/bazel-out/k8-fastbuild/bin/example.wasm ./
cd ../
date_value=$(date)
git commit -a -m "$date_value"
git push pubudu master
