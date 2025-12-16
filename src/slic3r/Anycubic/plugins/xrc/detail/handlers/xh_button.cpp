#include "xh_button.hpp"
#include "common.hpp"

#include <slic3r/GUI/Widgets/Button.hpp>

wxIMPLEMENT_DYNAMIC_CLASS(ButtonResourceHandler, wxXmlResourceHandler);

ButtonResourceHandler::ButtonResourceHandler() : wxXmlResourceHandler() {
  AddWindowStyles();
}

wxObject *ButtonResourceHandler::DoCreateResource() {
  XRC_MAKE_INSTANCE(button, Button);
  button->Create(m_parentAsWindow, GetText(wxT("label")), "", GetStyle(), 0,
                 GetID());
  SetupWindow(button);
  ButtonStyle style = ButtonStyle::Regular;
  XRC_PARSE_CLASS_ENUM(style, GetParamValue(wxT("ex_style")), Regular, Confirm,
                       Alert, Disabled);
  ButtonType extype = ButtonType::Compact;
  XRC_PARSE_CLASS_ENUM(extype, GetParamValue(wxT("ex_type")), Compact, Window,
                       Choice, Parameter, Expanded);
  button->SetStyle(style, extype);
  return button;
}

bool ButtonResourceHandler::CanHandle(wxXmlNode *node) {
  return IsOfClass(node, wxT("Button"));
}
