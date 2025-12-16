#include "xh_checkbox.hpp"
#include "common.hpp"
#include "slic3r/GUI/Widgets/CheckBox.hpp"

wxIMPLEMENT_DYNAMIC_CLASS(CheckBoxResourceHandler, wxToggleButtonXmlHandler);

wxObject *CheckBoxResourceHandler::DoCreateResource() {
  XRC_MAKE_INSTANCE_PARAMS(control, CheckBox, m_parentAsWindow, GetID());

  wxToggleButtonXmlHandler::DoCreateToggleButton(control);

  SetupWindow(control);
#ifdef __WXOSX__
  control->Enable(GetBool(wxT("enabled"), true));
#endif
  return control;
}

bool CheckBoxResourceHandler::CanHandle(wxXmlNode *node) {
  return IsOfClass(node, wxT("CheckBox"));
}
