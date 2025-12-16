#pragma once

#include <wx/xrc/xh_tglbtn.h>

class CheckBoxResourceHandler : public wxToggleButtonXmlHandler {
public:
  CheckBoxResourceHandler() = default;
  ~CheckBoxResourceHandler() override = default;

private:
  DECLARE_DYNAMIC_CLASS(CheckBoxResourceHandler);
  wxObject *DoCreateResource() override;
  bool CanHandle(wxXmlNode *node) override;
};
