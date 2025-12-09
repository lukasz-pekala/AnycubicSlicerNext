#include "xrc_pluigns.hpp"
// 自定义控件文件列表--如果添加自定义控件，需要在这包含头文件
#include "slic3r/GUI/Widgets/AMSControl.hpp"
#include "slic3r/GUI/Widgets/Button.hpp"
#include "slic3r/GUI/Widgets/ErrorMsgStaticText.hpp"

#include <wx/xrc/xmlres.h>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/tuple.hpp>

namespace Slic3r::GUI {

// // 自定义控件列表--如果添加自定义控件，需要在这添加到CONTORL_LIST宏中
// // 这里的类型要求有默认构造函数
// #define CONTORL_LIST (ErrorMsgStaticText)

// #define HANDLER_TYPE_NAME(type) BOOST_PP_CAT(ControlHandler_, type)

// #define CONTROL_HANDLER(r, nil, type)                                          \
//   struct HANDLER_TYPE_NAME(type) : public wxXmlResourceHandler {               \
//     wxObject *DoCreateResource() override { return new type; }                 \
//     bool CanHandle(wxXmlNode *node) override {                                 \
//       return IsOfClass(node, wxT(BOOST_PP_STRINGIZE(type)));                   \
//     }                                                                          \
//     DECLARE_DYNAMIC_CLASS(HANDLER_TYPE_NAME(type));                            \
//   };                                                                           \
//   wxIMPLEMENT_DYNAMIC_CLASS(HANDLER_TYPE_NAME(type), wxXmlResourceHandler);

// BOOST_PP_SEQ_FOR_EACH(CONTROL_HANDLER, nil, BOOST_PP_TUPLE_TO_SEQ(CONTORL_LIST))

#define REGISTER_HANDLER(r, nil, type)                                         \
  wxXmlResource::Get()->AddHandler(new HANDLER_TYPE_NAME(type));

XrcPlugin::XrcPlugin(Anycubic::Plugins::PluginHost *host) {
  // BOOST_PP_SEQ_FOR_EACH(REGISTER_HANDLER, nil,
  //                       BOOST_PP_TUPLE_TO_SEQ(CONTORL_LIST));
}
XrcPlugin::~XrcPlugin() {}
} // namespace Slic3r::GUI