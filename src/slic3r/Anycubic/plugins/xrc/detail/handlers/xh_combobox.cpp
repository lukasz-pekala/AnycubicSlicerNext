#include "xh_combobox.hpp"
#include "common.hpp"

#include <slic3r/GUI/Widgets/ComboBox.hpp>

#include "wx/xml/xml.h"

wxObject *ComboBoxResourceHandler::DoCreateResource() {
  if (m_class == wxT("ComboBox")) {
    m_insideBox = true;
    CreateChildrenPrivately(nullptr, GetParamNode(wxT("content")));
    XRC_MAKE_INSTANCE_PARAMS(control, ComboBox, m_parentAsWindow, GetID());
    int selection = static_cast<int>(GetLong(wxT("selection"), -1));
    if (selection != -1) {
      control->SetSelection(selection);
    }
    SetupWindow(control);
    for (auto x : strList) {
      control->Append(x);
    }
    return control;
  } else {
    strList.Add(GetNodeText(m_node, wxXRC_TEXT_NO_ESCAPE));
    return nullptr;
  }
}

bool ComboBoxResourceHandler::CanHandle(wxXmlNode *node) {
  return (IsOfClass(node, wxT("ComboBox")) ||
          (m_insideBox && node->GetName() == wxT("item")));
}
