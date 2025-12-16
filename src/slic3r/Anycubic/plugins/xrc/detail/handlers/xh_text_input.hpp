#pragma once
#include <wx/xrc/xmlres.h>

class TextInputResourceHandler : public wxXmlResourceHandler {
public:
  TextInputResourceHandler() = default;
  ~TextInputResourceHandler() override = default;
  wxObject *DoCreateResource() override;
  bool CanHandle(wxXmlNode *node) override;
};