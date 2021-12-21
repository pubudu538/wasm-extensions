#include <string>
#include <unordered_map>

// #include "absl/strings/str_cat.h"
#include "proxy_wasm_intrinsics.h"
#include "extensions/common/wasm/json_util.h"

using ::nlohmann::json;
using ::Wasm::Common::JsonArrayIterate;
using ::Wasm::Common::JsonGetField;
using ::Wasm::Common::JsonObjectIterate;
using ::Wasm::Common::JsonValueAs;

class ExampleRootContext : public RootContext {
public:
  explicit ExampleRootContext(uint32_t id, std::string_view root_id) : RootContext(id, root_id) {}

  bool onStart(size_t) override;
  bool onConfigure(size_t) override;
};

class ExampleContext : public Context {
public:
  explicit ExampleContext(uint32_t id, RootContext *root) : Context(id, root) {}

  FilterHeadersStatus onRequestHeaders(uint32_t headers, bool end_of_stream) override;
  void onDone() override;
};
static RegisterContextFactory register_ExampleContext(CONTEXT_FACTORY(ExampleContext),
                                                      ROOT_FACTORY(ExampleRootContext));

FilterHeadersStatus ExampleContext::onRequestHeaders(uint32_t headers, bool end_of_stream) {
  logInfo(std::string("onRequdestsHeaders ") + std::to_string(id()));
  auto path = getRequestHeader(":path");
  logInfo(std::string("header path ") + std::string(path->view()));
  addResponseHeader("X-Wasmss-custom", "FOO");
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

  auto configuration_data = getBufferBytes(WasmBufferType::PluginConfiguration, 0, config_size);

  logInfo("onConfigure.......");
  logInfo(configuration_data->view());


  // Parse configuration JSON string.
  auto result = ::Wasm::Common::JsonParse(configuration_data->view());
  if (!result.has_value()) {
    logWarn("cannot parse plugin configuration JSON string: ");
    return false;
  } else {
    logInfo("onConfigure....... - Parsing done@@@!!");
  }


  return true;
}