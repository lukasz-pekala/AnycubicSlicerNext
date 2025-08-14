set(NOSHARED no-shared)
if(CMAKE_HOST_WIN32)
    set(build_type "VC-WIN64A-masm")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(build_type "debug-VC-WIN64A-masm")
    endif()
    set(configure_cmd perl Configure)
    set(build_cmd set CL=/MP && nmake)
    set(install_cmd nmake install_sw)
    set(copy_cmd  COMMAND ${CMAKE_COMMAND} -E copy ms/applink.c ${DESTDIR}/usr/local/include/openssl/applink.c)
elseif(CMAKE_HOST_APPLE)
 
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        set(build_type "darwin64-arm64-cc")
    else()
        set(build_type "darwin64-x86_64-cc")
    endif()
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(build_type "debug-${build_type}")
    endif()
    set(osxmin -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set(configure_cmd perl Configure)
    set(build_cmd make "-j${NPROC}")
    set(install_cmd make install_sw)
    set(copy_cmd  COMMAND ${CMAKE_COMMAND} -E echo "non winodws")
else()
    set(build_type "linux-x86_64")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(build_type "debug-${build_type}")
    endif()
    
    set(configure_cmd ./Configure)
    set(build_cmd make "-j${NPROC}")
    set(install_cmd make install_sw)
    set(copy_cmd  COMMAND ${CMAKE_COMMAND} -E echo "non winodws")
endif()
set(URL "https://github.com/openssl/openssl/archive/OpenSSL_1_1_1w.tar.gz")
PROXY(new_URL URL)
ExternalProject_Add(dep_OpenSSL
    #EXCLUDE_FROM_ALL ON
    URL ${new_URL}
    URL_HASH SHA256=2130E8C2FB3B79D1086186F78E59E8BC8D1A6AEDF17AB3907F4CB9AE20918C41
    # URL "https://github.com/openssl/openssl/archive/refs/tags/openssl-3.1.2.tar.gz"
    # URL_HASH SHA256=8c776993154652d0bb393f506d850b811517c8bd8d24b1008aef57fbe55d3f31
    DOWNLOAD_DIR ${DEP_DOWNLOAD_DIR}/OpenSSL
    LOG_BUILD OFF
    LOG_TEST OFF
    DOWNLOAD_EXTRACT_TIMESTAMP true
	CONFIGURE_COMMAND  ${configure_cmd} ${build_type} ${NOSHARED} 
        "--openssldir=${DESTDIR}"
        "--prefix=${DESTDIR}"
        no-dynamic-engine ${osxmin}
    BUILD_IN_SOURCE ON
    BUILD_COMMAND ${build_cmd}
    INSTALL_COMMAND ${install_cmd}
)

ExternalProject_Add_Step(dep_OpenSSL install_cmake_files
    DEPENDEES install

    COMMAND ${CMAKE_COMMAND} -E copy_directory openssl "${DESTDIR}${CMAKE_INSTALL_LIBDIR}/cmake/openssl"
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
)
