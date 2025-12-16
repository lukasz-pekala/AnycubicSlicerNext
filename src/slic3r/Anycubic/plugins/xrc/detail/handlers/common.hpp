#pragma once

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>

#define STYLE_APPLY(r, data, elem)                                             \
  {                                                                            \
    using StyleType = std::decay_t<decltype(BOOST_PP_TUPLE_ELEM(0, data))>;    \
    if (BOOST_PP_TUPLE_ELEM(1, data) == wxT(BOOST_PP_STRINGIZE(elem))) {       \
      if constexpr (std::is_enum_v<StyleType>) {                               \
        BOOST_PP_TUPLE_ELEM(0, data) = StyleType::elem;                        \
      }                                                                        \
      break; /*中止*/                                                          \
    }                                                                          \
  }

/**
 * @brief 将字符串转换为枚举值
 *  @param value 目标变量
 *  @param text  输入字串变量
 *  @param ...   枚举值
 */
#define XRC_PARSE_CLASS_ENUM(value, text, ...)                                 \
  do {                                                                         \
    wxString str = text;                                                       \
    BOOST_PP_SEQ_FOR_EACH(STYLE_APPLY, (value, str),                           \
                          BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))               \
  } while (false)

#define XRC_MAKE_INSTANCE_PARAMS(variable, classname, ...)                     \
  classname *variable = nullptr;                                               \
  if (m_instance)                                                              \
    variable = wxStaticCast(m_instance, classname);                            \
  if (!variable)                                                               \
    variable = new classname(__VA_ARGS__);                                     \
  if (GetBool(wxT("hidden"), 0) == 1)                                          \
    variable->Hide();