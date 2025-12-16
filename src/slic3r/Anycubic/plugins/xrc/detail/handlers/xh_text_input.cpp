#include "xh_text_input.hpp"

#include "common.hpp"

#include <slic3r/GUI/Widgets/TempInput.hpp>

wxObject *TextInputResourceHandler::DoCreateResource() {
  XRC_MAKE_INSTANCE(control, TempInput);
  control->Create(m_parentAsWindow, GetText(wxT("text")));
  control->SetId(GetID());
  SetupWindow(control);
  return control;
}

bool TextInputResourceHandler::CanHandle(wxXmlNode *node) {
  return IsOfClass(node, wxT("TextInput"));
}
