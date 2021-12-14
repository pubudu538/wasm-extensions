cd example/
bazel build //:example.wasm
cp bazel-bin/example.wasm ./
cd ../
date_value=$(date)
git commit -a -m "$date_value"
git push pubudu master
