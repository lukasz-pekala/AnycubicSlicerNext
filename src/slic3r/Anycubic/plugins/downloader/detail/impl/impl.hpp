#pragma once
namespace Anycubic::Plugins {
struct PluginHost;
}
bool is_test_env(Anycubic::Plugins::PluginHost *host);

bool is_china_env(Anycubic::Plugins::PluginHost *host);
