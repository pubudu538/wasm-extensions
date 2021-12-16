#include "plugin.h"

// Boilderplate code to register the extension implementation.
static RegisterContextFactory register_Example(CONTEXT_FACTORY(PluginContext),
                                               ROOT_FACTORY(PluginRootContext));

bool PluginRootContext::onConfigure(size_t) { return true; }

FilterHeadersStatus PluginContext::onResponseHeaders(uint32_t, bool) {
  addResponseHeader("X-new-dsWasm-custom", "foo");
  LOG_INFO("Testing logging $$$");
  return FilterHeadersStatus::Continue;
}
