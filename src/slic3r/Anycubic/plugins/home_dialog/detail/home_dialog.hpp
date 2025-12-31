#pragma once

#include "plugins_manager/plugins_manager.hxx"

#include <boost/preprocessor/cat.hpp>

#include "plugins_base/plugins.hxx"
#include "plugins_base/plugins_base.hxx"
#include <wx/event.h>
#include <vector>

#define PLGUINS_NAME home_dialog
#define PLUGIN_NAME_STR BOOST_PP_STRINGIZE(PLGUINS_NAME)


wxDECLARE_EVENT(EVT_THREAD_CHANGE_TO_MAIN, wxCommandEvent);


struct parmObj
{
    wxString plugin_name;
    wxString title;
    wxString icon;
    wxString xrcName;
    wxString xrc;
    wxSize panelSize;
    bool    isShowModal;
};

class HomeDialog : public Anycubic::Plugins::Plugin,public wxEvtHandler
{
public:
    HomeDialog(Anycubic::Plugins::PluginHost* host);
    virtual ~HomeDialog() {}
    

public:
  wxWindow* GetTopParentWindow();


private:

  bool is_test_env(void) const;
  bool show_dialog(const wxString& plugin_name, const wxString& title,
                    const wxString &icon, const wxString &xrcName,
                    const wxString *xrc,wxSize *panelSize,bool isShowModal);
  void create_dialog(wxCommandEvent& evt);

  void show_message_dialog(wxWindow* parent, const wxString& message, const wxString& caption = wxEmptyString, long style = wxOK);

  void show_rich_message_dialog(wxWindow* parent, const wxString& message, const wxString& caption = wxEmptyString, long style = wxOK);
  void show_error_dialog(wxWindow* parent, const wxString& msg, bool courier_font);


  void OnCloudMqttEvent(wxCommandEvent& event);

 private:
  // Anycubic::Plugins::Plugin interface
  const char *Name(void) override { return PLUGIN_NAME_STR; }
  bool Start(void) override { return true; }
  void Stop(void) override {}
  bool        AttachEvt(wxEvtHandler* evt) override;
  bool        DetachEvt(wxEvtHandler* evt) override;
  bool BindEvt(class wxPanel* panel, wxWindow* parent = nullptr,
                   wxString *bmp = nullptr) override {
    return false;
  }
  bool CreateWebview(wxWebView *view, wxWindow *parent = nullptr,
                     wxString *bmp = nullptr) override {
    return false;
  }
  void Destroy(void) override { delete this; }

private:
  Anycubic::Plugins::PluginHost *host_;
  struct Tab {
    wxString pluginName;
    wxString title;
    wxString iconName;
    wxString xrcName;
    wxString xrc;
  };
  std::vector<Tab> tabs_;

  std::vector<wxEvtHandler*> m_evt_list;
  std::mutex                 mtx_;
};
