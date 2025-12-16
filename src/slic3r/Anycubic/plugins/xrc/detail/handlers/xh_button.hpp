#pragma once
#include <wx/xrc/xmlres.h>

class ButtonResourceHandler : public wxXmlResourceHandler {
public:
  ButtonResourceHandler();
  ~ButtonResourceHandler() override = default;

private:
  wxObject *DoCreateResource() override;
  bool CanHandle(wxXmlNode *node) override;
  DECLARE_DYNAMIC_CLASS(ButtonResourceHandler)
};