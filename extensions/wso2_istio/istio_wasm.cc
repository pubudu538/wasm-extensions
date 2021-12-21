#include "extensions/wso2_istio/istio_wasm.h"

#include <string>
#include <unordered_map>

#include "absl/strings/str_cat.h"
#include "extensions/common/wasm/json_util.h"
#include "proxy_wasm_intrinsics.h"

#include "proxy_wasm_intrinsics_lite.pb.h"
#include "google/protobuf/util/json_util.h"


#include "extensions/wso2_istio/echo/echo.pb.h"

static constexpr char EchoServerServiceName[] = "echo.EchoServer";
static constexpr char SayHelloMethodName[] = "SayHello";

//using google::protobuf::util::JsonParseOptions;
////using google::protobuf::util::error::Code;
//using google::protobuf::util::Status;

using echo::EchoRequest;
using echo::EchoReply;
using echo::Config;

using ::Wasm::Common::JsonValueAs;

static RegisterContextFactory register_ExampleContext(
    CONTEXT_FACTORY(ExampleContext), ROOT_FACTORY(ExampleRootContext));

FilterHeadersStatus ExampleContext::onRequestHeaders(uint32_t headers,
                                                     bool end_of_stream) {
  logInfo(std::string("onRequdestsHeaders ") + std::to_string(id()));
  auto path = getRequestHeader(":path");
  logInfo(std::string("header path ") + std::string(path->view()));
  addResponseHeader("X-Wasmss-custom", "FOO");

  logInfo("opa_host_->view()");

  rootContext()->check();
  return FilterHeadersStatus::Continue;
}

void ExampleContext::onDone() { logInfo("onDone " + std::to_string(id())); }

bool ExampleRootContext::onStart(size_t) {
  logInfo("onStart");
  return true;
}

FilterHeadersStatus ExampleRootContext::check() {
  logInfo("oncheeeeee000 -------");
  logInfo(opa_host_);
  return FilterHeadersStatus::Continue;

//   std::string jwt_string;
//   if (!getValue(
//           {"metadata", "filter_metadata", "envoy.filters.http.jwt_authn",
//           "my_payload", "sub"}, &jwt_string)) {
//     LOG_ERROR(std::string("filter_metadata Error ") + std::to_string(id()));
//   }
//
//   LOG_INFO(">>>>>>>>>>>>>  Calling GRPC for sub:" + jwt_string);
//   ExampleRootContext *a = dynamic_cast<ExampleRootContext*>(rootContext());
//   GrpcService grpc_service;
//   grpc_service.mutable_envoy_grpc()->set_cluster_name(a->opa_host_);
//   std::string grpc_service_string;
//   grpc_service.SerializeToString(&grpc_service_string);
//
//   EchoRequest request;
//   request.set_name(jwt_string);
//   std::string st2r = request.SerializeAsString();
//   HeaderStringPairs initial_metadata;
//   initial_metadata.push_back(std::pair("parent", "bar"));
//   auto res =  rootContext()->grpcCallHandler(grpc_service_string,
//   EchoServerServiceName, SayHelloMethodName, initial_metadata, st2r, 1000,
//                               std::unique_ptr<GrpcCallHandlerBase>(new
//                               MyGrpcCallHandler(this)));
//
//   if (res != WasmResult::Ok) {
//     LOG_ERROR("Calling gRPC server failed: " + toString(res));
//   }
//
//   return FilterHeadersStatus::StopIteration;
}

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

  auto j = result.value();
  auto it = j.find("clustername");
  if (it != j.end()) {
    auto opa_host_val = JsonValueAs<std::string>(it.value());
    opa_host_ = opa_host_val.first.value();
    logInfo(opa_host_);
  } else {
    LOG_WARN(
        absl::StrCat("opa service host must be provided in plugin "
                     "configuration JSON string: ",
                     configuration_data->view()));
    return false;
  }

  return true;
}

class MyGrpcCallHandler : public GrpcCallHandler<google::protobuf::Value> {
  public:
   MyGrpcCallHandler(ExampleContext *context) { context_ = context;  }

   void onSuccess(size_t body_size) override {
     LOG_INFO("GRPC call SUCCESS");
     WasmDataPtr response_data =
     getBufferBytes(WasmBufferType::GrpcReceiveBuffer, 0, body_size); const
     EchoReply& response = response_data->proto<EchoReply>(); LOG_INFO("got gRPC Response: " + response.message());

     context_->setEffectiveContext();

     auto res = addRequestHeader("isAdmin", response.message());
     if (res != WasmResult::Ok) {
       LOG_ERROR("Modifying Header data failed: " + toString(res));
     }
     continueRequest();
   }

   void onFailure(GrpcStatus status) override {
     LOG_INFO(" GRPC call FAILURE ");
     auto p = getStatus();
     LOG_DEBUG(std::string("failure ") +
     std::to_string(static_cast<int>(status)) +
              std::string(p.second->view()));
     context_->setEffectiveContext();
     closeRequest();
   }

  private:
   ExampleContext *context_;

 };