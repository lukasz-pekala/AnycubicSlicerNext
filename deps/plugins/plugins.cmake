AnycubicSlicer_add_cmake_project(
    OpenPlugins
    GIT_REPOSITORY "http://10.0.169.101/qprj/fdm/open-plugins.git"
    GIT_TAG "develop"
    GIT_SHALLOW ON
    DEPENDS ${WXWIDGETS_PKG} ${OPENSSL_PKG} dep_Boost ${CURL_PKG}
    CMAKE_ARGS
        -DENABLE_I18N=OFF
)