#include "home_dialog.hpp"

#include <plugins_base/funcation.hxx>

#include "AcDialog.hpp"
#include <slic3r/GUI/GUI_App.hpp>
#include <slic3r/GUI/MainFrame.hpp>
#include <slic3r/GUI/MsgDialog.hpp>


wxDEFINE_EVENT(EVT_THREAD_CHANGE_TO_MAIN, wxCommandEvent);


HomeDialog::HomeDialog(Anycubic::Plugins::PluginHost *host)
    : host_(host) {
  auto router = host_->Router();
  assert(router != nullptr);

  router->REGISTER_FUNCATION(HomeDialog, show_dialog);

  this->Bind(EVT_THREAD_CHANGE_TO_MAIN, &HomeDialog::create_dialog, this);

  router->REGISTER_FUNCATION(HomeDialog, show_rich_message_dialog);
  router->REGISTER_FUNCATION(HomeDialog, show_message_dialog);
  router->REGISTER_FUNCATION(HomeDialog, show_error_dialog);


}

void HomeDialog::show_error_dialog(wxWindow* parent, const wxString& msg, bool courier_font) 
{
    ErrorDialog showdialog(parent ? parent : wxGetApp().mainframe, msg, courier_font);
    showdialog.ShowModal();

}

void HomeDialog::show_rich_message_dialog(wxWindow* parent,const wxString& message,const wxString& caption,long style)
{
    RichMessageDialog dialog(parent ? parent : wxGetApp().mainframe, message, caption, style);
    dialog.Bind(wxEVT_CLOSE_WINDOW, [&dialog](wxCloseEvent& event) { dialog.EndModal(wxID_NO); });
    dialog.SetOKLabel(_("OK"));
    dialog.ShowModal();
}

void HomeDialog::show_message_dialog(wxWindow* parent, const wxString& message, const wxString& caption, long style)
{
    MessageDialog dlg(parent ? parent : wxGetApp().mainframe, message, caption, style);
    dlg.ShowModal();

}

bool HomeDialog::is_test_env(void) const
{
    
    return Anycubic::Plugins::dispatch_call<bool>(host_, "utility", "is_test_env");
}

wxWindow* HomeDialog::GetTopParentWindow()
{
    wxWindow* active = wxWindow::FindFocus();
    while (active && !dynamic_cast<wxTopLevelWindow*>(active))
        active = active->GetParent();

    if (!active) {
        return static_cast<wxWindow*>(Slic3r::GUI::wxGetApp().mainframe);
    }
    return active;
}

void HomeDialog::create_dialog(wxCommandEvent& evt)
{ 
    parmObj* parm = static_cast<parmObj*>(evt.GetClientData());
    if (parm) {
        ACShowDialog* dialog = new ACShowDialog(GetTopParentWindow(), is_test_env(), parm->panelSize);
        if (!parm->title.empty()) {
            dialog->SetTitle(parm->title);
        }
        if (!parm->icon.empty()) {
            dialog->SetIcon(parm->icon);
        }
        wxPanel* panel = host_->CreatePanel(dialog, parm->xrcName, parm->xrc);
        if (panel == nullptr) {
            delete parm;
            return;
        }
        dialog->ReplacePanel(panel);

        host_->GetPlugin(parm->plugin_name.ToStdString().c_str())->BindEvt(panel, dialog);
        int result = -1;
        if (parm->isShowModal) {
            result = dialog->ShowModal();
        } else {
            dialog->Raise();
            dialog->Centre();
            dialog->Show();
        }

        delete parm;
    }


}


bool HomeDialog::show_dialog(const wxString &plugin_name,const wxString &title,const wxString &icon,
                             const wxString& xrcName,const wxString* xrc,wxSize *panelSize,bool isShowModal)
{
  assert(xrc != nullptr && xrc->IsEmpty() == false);
  assert(plugin_name.IsEmpty() == false);
  if (!host_->HasPlugin(plugin_name.utf8_string().c_str())) {
    return false;
  }

  wxCommandEvent* main_evt = new wxCommandEvent(EVT_THREAD_CHANGE_TO_MAIN);
  parmObj *obj      = new parmObj;
  obj->plugin_name = plugin_name;
  obj->title               = title;
  obj->icon                = icon;
  obj->xrcName             = xrcName;
  obj->xrc                 = *xrc;
  obj->panelSize           = *panelSize;
  obj->isShowModal         = isShowModal;
  main_evt->SetClientData(obj);

  wxQueueEvent(this, main_evt);

  return true;
}


bool HomeDialog::AttachEvt(wxEvtHandler* evt)
{
    std::lock_guard<std::mutex> lock(mtx_);

    if (std::find(m_evt_list.begin(), m_evt_list.end(), evt) != m_evt_list.end())
        return false; 
    m_evt_list.push_back(evt);
    return true;
}

bool HomeDialog::DetachEvt(wxEvtHandler* evt)
{
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = std::find(m_evt_list.begin(), m_evt_list.end(), evt);
    if (it == m_evt_list.end())
        return false;
    m_evt_list.erase(it, m_evt_list.end());

    return true;
}

void HomeDialog::OnCloudMqttEvent(wxCommandEvent& event)
{
    wxPropagationDisabler       disablePropagation(event);
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& e : m_evt_list) {
        e->ProcessEvent(event);
    }
}
