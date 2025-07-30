set(_wx_toolkit "")
set(_wx_private_font "-DwxUSE_PRIVATE_FONTS=1")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(_gtk_ver 2)

    if (DEP_WX_GTK3)
        set(_gtk_ver 3)
    endif ()

    set(_wx_toolkit "-DwxBUILD_TOOLKIT=gtk${_gtk_ver}")
endif()



# Note: The flatpak build builds wxwidgets separately due to CI size constraints.
# ANY CHANGES MADE IN HERE MUST ALSO BE REFLECTED IN `flatpak/io.github.SoftFever.OrcaSlicer.yml`.
# ** THIS INCLUDES BUILD ARGS. **
# ...if you can find a way around this size limitation, be my guest.

orcaslicer_add_cmake_project(
    wxWidgets
    GIT_REPOSITORY "http://192.168.2.20/r/~liuchunlin/dep_orca_wxwidgets.git"
    GIT_SHALLOW ON
    DEPENDS ${PNG_PKG} ${ZLIB_PKG} ${EXPAT_PKG} ${TIFF_PKG} ${JPEG_PKG}
    CMAKE_ARGS
        -DwxBUILD_PRECOMP=ON
        ${_wx_toolkit}
        "-DCMAKE_DEBUG_POSTFIX:STRING="
        -DwxBUILD_DEBUG_LEVEL=0
        -DwxBUILD_SAMPLES=OFF
        -DwxBUILD_SHARED=ON
        -DwxUSE_MEDIACTRL=ON
        -DwxUSE_DETECT_SM=OFF
        -DwxUSE_UNICODE=ON
        ${_wx_private_font}
        -DwxUSE_OPENGL=ON
        -DwxUSE_WEBREQUEST=ON
        -DwxUSE_WEBVIEW=OFF
        -DwxUSE_REGEX=builtin
        -DwxUSE_LIBXPM=builtin
        -DwxUSE_LIBSDL=OFF
        -DwxUSE_XTEST=OFF
        -DwxUSE_STC=OFF
        -DwxUSE_AUI=ON
        -DwxUSE_LIBPNG=sys
        -DwxUSE_ZLIB=sys
        -DwxUSE_LIBJPEG=sys
        -DwxUSE_LIBTIFF=sys
        -DwxUSE_NANOSVG=OFF
        -DwxUSE_EXPAT=sys
)

if (MSVC)
    add_debug_dep(dep_wxWidgets)
endif ()
