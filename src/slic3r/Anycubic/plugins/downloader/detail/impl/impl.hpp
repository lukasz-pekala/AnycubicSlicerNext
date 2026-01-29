#pragma once
namespace Anycubic::Plugins {
struct PluginHost;
}
class wxString;

bool is_test_env(Anycubic::Plugins::PluginHost *host);

bool is_china_env(Anycubic::Plugins::PluginHost *host);

bool show_progress_dialog(Anycubic::Plugins::PluginHost *host, const char *name,
                          const wxString &title);