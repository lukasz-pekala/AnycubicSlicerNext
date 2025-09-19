#pragma once
#include <wx/xrc/xmlres.h>
class DPIFrameHandler : public wxXmlResourceHandler {
public:
  DPIFrameHandler();

private:
  wxObject *DoCreateResource() override;
  bool CanHandle(wxXmlNode *node) override;
  DECLARE_DYNAMIC_CLASS(DPIFrameHandler)
public:
};

class DPIDialogHandler : public wxXmlResourceHandler {
public:
  DPIDialogHandler();

private:
  wxObject *DoCreateResource() override;
  bool CanHandle(wxXmlNode *node) override;
  DECLARE_DYNAMIC_CLASS(DPIDialogHandler)
public:
};