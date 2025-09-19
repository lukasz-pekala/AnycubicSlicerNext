#include "dpi_aware.hpp"

#include <slic3r/GUI/GUI_Utils.hpp>
// demo
struct DPIFrame : public Slic3r::GUI::DPIFrame {
  DPIFrame(wxWindow *parent, wxWindowID id, const wxString &title,
           const wxPoint &pos = wxDefaultPosition,
           const wxSize &size = wxDefaultSize,
           long style = wxDEFAULT_FRAME_STYLE,
           const wxString &name = wxFrameNameStr)
      : Slic3r::GUI::DPIFrame(parent, id, title, pos, size, style, name) {}
  void on_dpi_changed(const wxRect &suggested_rect) {}
};
struct DPIDialog : public Slic3r::GUI::DPIDialog {
  DPIDialog(wxWindow *parent, wxWindowID id, const wxString &title,
            const wxPoint &pos = wxDefaultPosition,
            const wxSize &size = wxDefaultSize,
            long style = wxDEFAULT_FRAME_STYLE,
            const wxString &name = wxFrameNameStr)
      : Slic3r::GUI::DPIDialog(parent, id, title, pos, size, style, name) {}
  void on_dpi_changed(const wxRect &suggested_rect) {}
};

wxObject *DPIFrameHandler::DoCreateResource() {

  DPIFrame *frame = nullptr;
  if (m_instance)
    frame = wxStaticCast(m_instance, DPIFrame);
  if (!frame)
    frame =
        new DPIFrame(m_parentAsWindow, GetID(), GetText(wxT("title")),
                     wxDefaultPosition, wxDefaultSize,
                     GetStyle(wxT("style"), wxDEFAULT_FRAME_STYLE), GetName());
  if (GetBool(wxT("hidden"), 0) == 1)
    frame->Hide();

  if (HasParam(wxT("size")))
    frame->SetClientSize(GetSize(wxT("size"), frame));
  if (HasParam(wxT("pos")))
    frame->Move(GetPosition());
  if (HasParam(wxT("icon")))
    frame->SetIcons(GetIconBundle(wxT("icon"), wxART_FRAME_ICON));

  SetupWindow(frame);

  CreateChildren(frame);

  if (GetBool(wxT("centered"), false))
    frame->Centre();

  return frame;
}
bool DPIFrameHandler::CanHandle(wxXmlNode *node) {
  return IsOfClass(node, wxT("DPIFrame"));
}
DPIFrameHandler::DPIFrameHandler() {
  XRC_ADD_STYLE(wxSTAY_ON_TOP);
  XRC_ADD_STYLE(wxCAPTION);
  XRC_ADD_STYLE(wxDEFAULT_DIALOG_STYLE);
  XRC_ADD_STYLE(wxDEFAULT_FRAME_STYLE);
  XRC_ADD_STYLE(wxSYSTEM_MENU);
  XRC_ADD_STYLE(wxRESIZE_BORDER);
  XRC_ADD_STYLE(wxCLOSE_BOX);

  XRC_ADD_STYLE(wxFRAME_NO_TASKBAR);
  XRC_ADD_STYLE(wxFRAME_SHAPED);
  XRC_ADD_STYLE(wxFRAME_TOOL_WINDOW);
  XRC_ADD_STYLE(wxFRAME_FLOAT_ON_PARENT);
  XRC_ADD_STYLE(wxMAXIMIZE_BOX);
  XRC_ADD_STYLE(wxMINIMIZE_BOX);
  XRC_ADD_STYLE(wxSTAY_ON_TOP);

  XRC_ADD_STYLE(wxTAB_TRAVERSAL);
  XRC_ADD_STYLE(wxWS_EX_VALIDATE_RECURSIVELY);
  XRC_ADD_STYLE(wxFRAME_EX_METAL);
  XRC_ADD_STYLE(wxFRAME_EX_CONTEXTHELP);

  AddWindowStyles();
}
wxIMPLEMENT_DYNAMIC_CLASS(DPIFrameHandler, wxXmlResourceHandler);

wxObject *DPIDialogHandler::DoCreateResource() {

  DPIDialog *dlg = nullptr;
  if (m_instance)
    dlg = wxStaticCast(m_instance, DPIDialog);
  if (!dlg)
    dlg = new DPIDialog(m_parentAsWindow, GetID(), GetText(wxT("title")),
                        wxDefaultPosition, wxDefaultSize,
                        GetStyle(wxT("style"), wxDEFAULT_DIALOG_STYLE),
                        GetName());
  if (GetBool(wxT("hidden"), 0) == 1)
    dlg->Hide();

  if (HasParam(wxT("size")))
    dlg->SetClientSize(GetSize(wxT("size"), dlg));
  if (HasParam(wxT("pos")))
    dlg->Move(GetPosition());
  if (HasParam(wxT("icon")))
    dlg->SetIcons(GetIconBundle(wxT("icon"), wxART_FRAME_ICON));

  SetupWindow(dlg);

  CreateChildren(dlg);

  if (GetBool(wxT("centered"), false))
    dlg->Centre();

  return dlg;
}
bool DPIDialogHandler::CanHandle(wxXmlNode *node) {
  return IsOfClass(node, wxT("DPIDialog"));
}

DPIDialogHandler::DPIDialogHandler() {
  XRC_ADD_STYLE(wxSTAY_ON_TOP);
  XRC_ADD_STYLE(wxCAPTION);
  XRC_ADD_STYLE(wxDEFAULT_DIALOG_STYLE);
  XRC_ADD_STYLE(wxSYSTEM_MENU);
  XRC_ADD_STYLE(wxRESIZE_BORDER);
  XRC_ADD_STYLE(wxCLOSE_BOX);
  XRC_ADD_STYLE(wxDIALOG_NO_PARENT);

  XRC_ADD_STYLE(wxTAB_TRAVERSAL);
  XRC_ADD_STYLE(wxWS_EX_VALIDATE_RECURSIVELY);
  XRC_ADD_STYLE(wxDIALOG_EX_METAL);
  XRC_ADD_STYLE(wxMAXIMIZE_BOX);
  XRC_ADD_STYLE(wxMINIMIZE_BOX);
  XRC_ADD_STYLE(wxFRAME_SHAPED);
  XRC_ADD_STYLE(wxDIALOG_EX_CONTEXTHELP);

  AddWindowStyles();
}

wxIMPLEMENT_DYNAMIC_CLASS(DPIDialogHandler, wxXmlResourceHandler);