#pragma once
#include <common/log/log.hpp>


class wxString;
class wxWindow;

namespace Slic3r { namespace GUI {

class AnycubicContextPrivate;

class AnycubicContext{
public:
AnycubicContext();
~AnycubicContext();

bool AddWindow(const wxString&position,wxWindow* window);
void OnInitByApp();     ///< 在 new MainFrame 之前调用
void OnInitByGui();     ///< 在 new MainFrame 构造时调用
void OnFinishedByGui(); ///< GUI初始化完成后调用
void OnExitByGui();     ///< 在 MainFrame::shutdown() 之前
void OnExitByApp();     ///< 在 delete MainFrame 销毁之后

private:
    AnycubicContextPrivate* impl_;

};

}} // namespace Slic3r::GUI
