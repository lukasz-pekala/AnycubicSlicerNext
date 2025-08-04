
#include "AnycubicContext.hpp"



namespace Slic3r { namespace GUI {

AnycubicContext::AnycubicContext(){
}
AnycubicContext::~AnycubicContext(){
    
}

bool AnycubicContext::AddWindow(const wxString& position, wxWindow* window) {
    assert(window!=nullptr);
    return false;
}

void AnycubicContext::OnInitByApp() {}
void AnycubicContext::OnInitByGui() {
   
}
void AnycubicContext::OnFinishedByGui(){}
void AnycubicContext::OnExitByGui(){
    
}
void AnycubicContext::OnExitByApp(){}    


}} // namespace Slic3r::GUI
