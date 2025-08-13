orcaslicer_add_cmake_project(
    OpenPlugins
    GIT_REPOSITORY "http://10.0.169.101/qprj/fdm/open-plugins.git"
    GIT_TAG "main"
    GIT_SHALLOW ON
    DEPENDS ${WXWIDGETS_PKG} ${OPENSSL_PKG} dep_Boost
)