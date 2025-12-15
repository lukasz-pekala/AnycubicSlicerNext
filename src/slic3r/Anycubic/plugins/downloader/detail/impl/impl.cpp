#include "impl.hpp"

#include <plugins_base/funcation.hxx>

bool is_test_env(Anycubic::Plugins::PluginHost *host) {
  return dispatch_call<bool>(host, "utility", "is_test_env");
}

bool is_china_env(Anycubic::Plugins::PluginHost *host) {
  return dispatch_call<bool>(host, "utility", "is_china_env");
}
