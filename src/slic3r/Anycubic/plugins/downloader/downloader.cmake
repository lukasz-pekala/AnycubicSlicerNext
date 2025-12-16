list(APPEND PLUGINS_LIST downloader)
set(PLUGINS_LIST ${PLUGINS_LIST} PARENT_SCOPE)
anycubic_search_src(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/detail ${CMAKE_CURRENT_LIST_DIR}/detail/protocol)

# 有部代码用到了c++20的特性，套个娃解决
file(GLOB_RECURSE DOWNLOADER_IMPL_SRC ${CMAKE_CURRENT_LIST_DIR}/detail/impl/*.cpp ${CMAKE_CURRENT_LIST_DIR}/detail/impl/*.hpp)
add_library(downloader_impl STATIC ${DOWNLOADER_IMPL_SRC})
set(ENABLE_I18N OFF) # 使用主程序的i18n，插件里的不能启用
find_package(OpenPlugins CONFIG REQUIRED COMPONENTS constant plugins_manager plugins_base easy_log utility)
find_package(Boost REQUIRED CONFIG)
target_link_libraries(downloader_impl PUBLIC  OpenPlugins::constant OpenPlugins::plugins_manager OpenPlugins::plugins_base OpenPlugins::easy_log OpenPlugins::utility Boost::system)
target_compile_definitions(downloader_impl PRIVATE ENABLE_LOG_CHECK_ARGS=1 ENABLE_STRACE=1 FMT_HEADER_ONLY=1 MODULE_NAME="MainApp"  wxDEBUG_LEVEL=0)
target_compile_features(downloader_impl PRIVATE cxx_std_20)
target_include_directories(downloader_impl PRIVATE ${Boost_INCLUDE_DIRS})
anycubic_link(downloader_impl)