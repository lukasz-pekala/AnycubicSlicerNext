#include "home_dialog.hpp"

#include <plugins_base/funcation.hxx>

#include "AcDialog.hpp"
#include <slic3r/GUI/GUI_App.hpp>
#include <slic3r/GUI/MainFrame.hpp>


wxDEFINE_EVENT(EVT_THREAD_CHANGE_TO_MAIN, wxCommandEvent);


HomeDialog::HomeDialog(Anycubic::Plugins::PluginHost *host)
    : host_(host) {
  auto router = host_->Router();
  assert(router != nullptr);

  router->REGISTER_FUNCATION(HomeDialog, show_dialog);

  this->Bind(EVT_THREAD_CHANGE_TO_MAIN, &HomeDialog::create_dialog, this);
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

        if (parm->isShowModal) {
            dialog->ShowModal();
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
  if (host_->HasPlugin(plugin_name.utf8_string().c_str())) {
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
