
macro(anycubic_target_link link_to_target OUT_SDK_DLLS)
    include(${CMAKE_CURRENT_LIST_DIR}/LIBRARY_CONFIG.cmake OPTIONAL)
    string(REPLACE "." "" VERSION_CODE ${SLIC3R_VERSION})
    
    target_compile_definitions(${link_to_target} PRIVATE VERSION_CODE=${VERSION_CODE} ENABLE_LOG_CHECK_ARGS=1 ENABLE_STRACE=1 FMT_HEADER_ONLY=1)
    find_package(OpenPlugins CONFIG REQUIRED COMPONENTS ACWebView plugins_manager plugins_base easy_log utility)
    find_package(Boost REQUIRED CONFIG COMPONENTS json)
    list(APPEND SDK_LIBS_G OpenPlugins::ACWebView OpenPlugins::plugins_manager OpenPlugins::plugins_base OpenPlugins::easy_log OpenPlugins::utility ${ANYCUBIC_LINKS})
    target_link_libraries(${link_to_target} PUBLIC ${SDK_LIBS_G} Boost::json ${wxWidgets_LIBRARIES})
    target_compile_definitions(${link_to_target} PRIVATE MODULE_NAME="MainApp")
    target_include_directories(${link_to_target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    if(CMAKE_HOST_LINUX)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
        pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.1)
        pkg_check_modules(PSL REQUIRED libpsl)
        target_link_libraries(${link_to_target} PUBLIC  ${WEBKIT2GTK_LIBRARIES} ${GTK3_LIBRARIES} ${PSL_LIBRARIES})
    endif()

    if(NOT CMAKE_HOST_WIN32)
        # 提取wxWidgets_LIBRARIES中的路径库和库名用于搜索
        set(wxWidgets_LIBRARIES_PATH "")
        set(wxWidgets_LIBRARIES_NAME "wx_baseu_xml-3.1")
        foreach(lib ${wxWidgets_LIBRARIES})
            if(lib MATCHES "^-L")
                string(REPLACE "-L" "" path ${lib})
                list(APPEND wxWidgets_LIBRARIES_PATH ${path})
            elseif(lib MATCHES "^-l")
                string(REPLACE "-l" "" name ${lib})
                list(APPEND wxWidgets_LIBRARIES_NAME ${name})
            endif()
        endforeach()

        
        message("wxWidgets_LIBRARIES_PATH: ${wxWidgets_LIBRARIES_NAME}")

        foreach(name ${wxWidgets_LIBRARIES_NAME})
            find_library(wxWidgets_LIBRARY_${name} NAMES ${name} PATHS ${wxWidgets_LIBRARIES_PATH})
            if(wxWidgets_LIBRARY_${name})
                list(APPEND MODULE_LIST ${wxWidgets_LIBRARY_${name}})
            else()
                message(FATAL_ERROR "wxWidgets_LIBRARY_${name} not found")
            endif()
        endforeach()
    else()
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(POSFIX "d")

        endif()
        get_target_property(LINKED_LIBS ${link_to_target} LINK_LIBRARIES)

        foreach(lib ${LINKED_LIBS})
            if(TARGET ${lib})
                continue()
            endif()
            string(FIND ${lib} "wxmsw" wx_index)
            if(wx_index EQUAL -1)
                continue()
            endif()
            cmake_path(GET lib STEM lib_name)
            if(NOT lib_path)
                cmake_path(GET lib PARENT_PATH lib_path)
                set(lib_path ${lib_path} PARENT_SCOPE)
            endif()

            # 对lib_name进行分割，以_为分隔符
            string(REPLACE "_" ";" lib_name_list ${lib_name})
            list(GET lib_name_list 1 lib_name)
            list(APPEND MODULE_LIST "${lib_path}/wxmsw315u${POSFIX}_${lib_name}_vc_custom.dll")
        endforeach()
        foreach(item wxbase315u${POSFIX}_vc_custom.dll;wxbase315u${POSFIX}_net_vc_custom.dll;wxbase315u${POSFIX}_xml_vc_custom.dll)
            list(APPEND MODULE_LIST "${lib_path}/${item}")
        endforeach()
    endif()

    

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(_build_type "_DEBUG")
    else()
        set(_build_type "_RELEASE")
    endif()

    # Loop through each library in SDK_LIBD
    foreach(lib ${SDK_LIBS_G})
        if(NOT TARGET ${lib})
            message(FATAL_ERROR "Target ${lib} not found")
        endif()

        get_target_property(location_file ${lib} IMPORTED_LOCATION${_build_type})
        if(location_file)
            foreach(dll ${location_file})
                if(EXISTS ${dll})
                    list(APPEND MODULE_LIST ${dll})
                    message("++ sdk location_file: ${dll}")
                else()
                    message(FATAL_ERROR "${lib} ${dll} not found")
                endif()
            endforeach()
        endif()
        
        if(CMAKE_HOST_WIN32)
            if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
                get_target_property(location_file ${lib} PDB_NAME${_build_type})
                if(location_file)
                    if(location_file AND EXISTS ${location_file})
                        list(APPEND MODULE_LIST ${location_file})
                        message("++ sdk location_file: ${location_file}")
                    else()
                        message(FATAL_ERROR "${lib} ${location_file} not found")
                    endif()
                endif()
            endif()
        endif()
    endforeach()    
    set(OUT_SDK_DLLS ${MODULE_LIST} PARENT_SCOPE)
endmacro()




function(anycubic_create_link target src_directory des_directory)
    file(TO_NATIVE_PATH "${src_directory}" src)
    file(TO_NATIVE_PATH "${des_directory}" des)

    if(NOT TARGET ${target})
        message(FATAL_ERROR "Target ${target} not found")
    endif()
    if(NOT EXISTS ${src})
        message(FATAL_ERROR "path ${src_directory} not found")
    endif()

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
        add_custom_command(
            TARGET ${target} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${des}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            VERBATIM
        )
        add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${src}" "${des}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            VERBATIM
        )
    elseif(CMAKE_HOST_WIN32)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND if not exist "${des}" "(" mklink /J "${des}" "${src}" ")"
            VERBATIM
        )
    else()
        add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ln -sfn "${src}" "${des}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            VERBATIM)

    endif()
endfunction()


function(anycubic_copy_to_build_tree target)
    message("++ target: ${target}")
    set(des_directory "${OUTPUT_PATH}")
    if(CMAKE_HOST_APPLE)
        set(des_directory "${OUTPUT_PATH}/${target}.app/Contents/Frameworks/")
    endif()
    message("anycubic_copy_to_build_tree: ${des_directory}")

    if(NOT EXISTS ${des_directory})
        file(MAKE_DIRECTORY ${des_directory})
    endif()
    if(CMAKE_HOST_WIN32)
        list(FILTER OUT_SDK_DLLS INCLUDE REGEX "\\.dll$")
    elseif(CMAKE_HOST_APPLE)
        list(FILTER OUT_SDK_DLLS INCLUDE REGEX "\\.dylib$")
        set_target_properties(${target} PROPERTIES 
            MACOSX_BUNDLE TRUE
            INSTALL_RPATH "@executable_path/../Frameworks"
        )
    elseif(CMAKE_HOST_LINUX)
        list(FILTER OUT_SDK_DLLS INCLUDE REGEX "\\.so$")
    endif()
    
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${OUT_SDK_DLLS} "${des_directory}"
        VERBATIM)
    
endfunction()


macro(anycubic_link)
    list(APPEND ANYCUBIC_LINKS ${ARGN})
    set(ANYCUBIC_LINKS ${ANYCUBIC_LINKS} PARENT_SCOPE)
endmacro()



function(anycubic_search_src)
    foreach(directory ${ARGN})
        file(GLOB SRC_G "${directory}/*.h*" "${directory}/*.c" "${directory}/*.cpp")
        source_group(TREE ${directory} FILES ${SRC_G})
        list(APPEND ANYCUBIC_SUB_DIR_SOURCES ${SRC_G})
        if(CMAKE_HOST_APPLE)
            file(GLOB MM_G "${directory}/*.mm")
            list(APPEND ANYCUBIC_SUB_DIR_SOURCES ${MM_G})
        endif()
    endforeach()
    set(ANYCUBIC_SUB_DIR_SOURCES ${ANYCUBIC_SUB_DIR_SOURCES} PARENT_SCOPE)
endfunction()

file(GLOB_RECURSE CMAKE_G "${CMAKE_CURRENT_LIST_DIR}/*.cmake")
list(REMOVE_ITEM CMAKE_G "${CMAKE_CURRENT_LIST_DIR}/Anycubic.cmake")


anycubic_search_src(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/detail)
set(PLUGINS_LIST "")
foreach(cmake ${CMAKE_G})
    include(${cmake})
endforeach()



anycubic_plugins_generate_header(${PLUGINS_LIST})



