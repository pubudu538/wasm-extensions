#include <string>
#include <unordered_map>

#include "absl/strings/str_cat.h"
#include "extensions/common/wasm/json_util.h"
#include "proxy_wasm_intrinsics.h"

using ::nlohmann::json;
using ::Wasm::Common::JsonArrayIterate;
using ::Wasm::Common::JsonGetField;
using ::Wasm::Common::JsonObjectIterate;
using ::Wasm::Common::JsonValueAs;

class ExampleRootContext : public RootContext {
 public:
  explicit ExampleRootContext(uint32_t id, std::string_view root_id)
      : RootContext(id, root_id) {}

  bool onStart(size_t) override;
  bool onConfigure(size_t) override;
};

class ExampleContext : public Context {
 public:
  explicit ExampleContext(uint32_t id, RootContext *root) : Context(id, root) {}

  FilterHeadersStatus onRequestHeaders(uint32_t headers,
                                       bool end_of_stream) override;
  void onDone() override;
};
static RegisterContextFactory register_ExampleContext(
    CONTEXT_FACTORY(ExampleContext), ROOT_FACTORY(ExampleRootContext));

FilterHeadersStatus ExampleContext::onRequestHeaders(uint32_t headers,
                                                     bool end_of_stream) {
  logInfo(std::string("onRequdestsHeaders ") + std::to_string(id()));
  auto path = getRequestHeader(":path");
  logInfo(std::string("header path ") + std::string(path->view()));
  addResponseHeader("X-Wasmss-custom", "FOO");

  logInfo("opa_host_->view()");

  // logInfo(opa_host_->view());
  return FilterHeadersStatus::Continue;
}

void ExampleContext::onDone() { logInfo("onDone " + std::to_string(id())); }

bool ExampleRootContext::onStart(size_t) {
  logInfo("onStart");
  return true;
}

// bool ExampleRootContext::onConfigure(size_t) {
//   logInfo("onConfigure");
//   proxy_set_tick_period_milliseconds(1000); // 1 sec
//   return true;
// }

bool ExampleRootContext::onConfigure(size_t config_size) {
  LOG_INFO("onConfigure called");
  logInfo("onConfigure");
  // proxy_set_tick_period_milliseconds(1000); // 1 sec

  auto configuration_data =
      getBufferBytes(WasmBufferType::PluginConfiguration, 0, config_size);

  logInfo("onConfigure.......");
  logInfo(configuration_data->view());

  // Parse configuration JSON string.
  auto result = ::Wasm::Common::JsonParse(configuration_data->view());
  if (!result.has_value()) {
    LOG_WARN(absl::StrCat("cannot parse plugin configuration JSON string: ",
                          configuration_data->view()));
    return false;
  } 

  // Get OPA extension configuration
  // {
  //   "opa_service_host": "opa.default.svc.cluster.local",
  //   "opa_cluster_name": "outbound|8080||opa.default.svc.cluster.local",
  //   "check_result_cache_valid_sec": 10
  // }
  // Parse and get opa service host.

  auto j = result.value();
  auto it = j.find("clustername");
  if (it != j.end()) {
    auto opa_host_val = JsonValueAs<std::string>(it.value());
    // if (opa_host_val.second != Wasm::Common::JsonParserResultDetail::OK) {
    //   LOG_WARN(absl::StrCat(
    //       "cannot parse opa service host in plugin configuration JSON string: ",
    //       configuration_data->view()));
    //   return false;
    // }
    opa_host_ = opa_host_val.first.value();
    // logInfo(opa_host_);
  } else {
    LOG_WARN(
        absl::StrCat("opa service host must be provided in plugin "
                     "configuration JSON string: ",
                     configuration_data->view()));
    return false;
  }



  return true;
}