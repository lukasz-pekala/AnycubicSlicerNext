#pragma once

#include <atomic>

// clang-format off
#define EXPEND_INITIALIZER_BEGIN(type, value, ret_exprt)                       \
  static type value;                                                           \
  static std::atomic_int initialized(0); /*0 未初始化 1 已初始化,2 初始化中*/      \
if(initialized.load() == 1) {                                                  \
    ret_exprt;                                                                 \
    return true;                                                               \
  }                                                                            \
  int ret = 0;                                                                 \
  if (initialized.compare_exchange_strong(ret, 2)) {

#define EXPEND_INITIALIZER_END(ret_exprt)                                      \
  initialized.store(1);                                                        \
  }                                                                            \
  else {                                                                       \
    while (initialized.load() != 1) /* 等待初始化完成*/                          \
      ;                                                                        \
  }                                                                            \
  ret_exprt;                                                                   \
  return true;
// clang-format on