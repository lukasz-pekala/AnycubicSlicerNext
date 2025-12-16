#pragma once

#include <wx/xrc/xmlres.h>

class ComboBoxResourceHandler : public wxXmlResourceHandler {
public:
  ComboBoxResourceHandler() = default;
  ~ComboBoxResourceHandler() override = default;
  wxObject *DoCreateResource() override;
  bool CanHandle(wxXmlNode *node) override;

private:
  bool m_insideBox = false;
  wxArrayString strList;
};