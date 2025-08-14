
macro(anycubic_target_link link_to_target OUT_SDK_DLLS)
    include(${CMAKE_CURRENT_LIST_DIR}/LIBRARY_CONFIG.cmake OPTIONAL)
    string(REPLACE "." "" VERSION_CODE ${SLIC3R_VERSION})
    
    target_compile_definitions(${link_to_target} PRIVATE VERSION_CODE=${VERSION_CODE} ENABLE_LOG_CHECK_ARGS=1 ENABLE_STRACE=1)
    find_package(OpenPlugins CONFIG REQUIRED COMPONENTS ACWebView plugins_manager plugins_base easy_log utility)
    find_package(Boost REQUIRED CONFIG COMPONENTS json)
    list(APPEND SDK_LIBS_G OpenPlugins::ACWebView OpenPlugins::plugins_manager OpenPlugins::plugins_base OpenPlugins::easy_log OpenPlugins::utility)
    target_link_libraries(${link_to_target} PUBLIC ${SDK_LIBS_G} Boost::json ${wxWidgets_LIBRARIES})
    target_compile_definitions(${link_to_target} PRIVATE MODULE_NAME="MainApp")
    target_include_directories(${link_to_target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})


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
    get_target_property(des_directory ${target} RUNTIME_OUTPUT_DIRECTORY)
    if(CMAKE_HOST_APPLE)
        set(des_directory "${des_directory}${CMAKE_BUILD_TYPE}/${target}.app/Contents/Frameworks/")
    endif()
    if(NOT EXISTS ${des_directory})
        file(MAKE_DIRECTORY ${des_directory})
    endif()
    if(CMAKE_HOST_WIN32)
        list(FILTER OUT_SDK_DLLS INCLUDE REGEX "\\.dll$")
    elseif(CMAKE_HOST_APPLE)
        list(FILTER OUT_SDK_DLLS INCLUDE REGEX "\\.dylib$")
    else()
        list(FILTER OUT_SDK_DLLS INCLUDE REGEX "\\.so$")
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${OUT_SDK_DLLS} "${des_directory}"
        VERBATIM)
    set_target_properties(${target} PROPERTIES 
            MACOSX_BUNDLE TRUE
            INSTALL_RPATH "@executable_path/../Frameworks"
        )
endfunction()





macro(anycubic_search_src directory)
    file(GLOB_RECURSE SRC_G "${directory}/*.h*" "${directory}/*.c" "${directory}/*.cpp")
    list(APPEND ANYCUBIC_SUB_DIR_SOURCES ${SRC_G})
    if(CMAKE_HOST_APPLE)
        file(GLOB_RECURSE MM_G "${directory}/*.mm")
        list(APPEND ANYCUBIC_SUB_DIR_SOURCES ${MM_G})
    endif()
endmacro()

file(GLOB_RECURSE CMAKE_G "${CMAKE_CURRENT_LIST_DIR}/*.cmake")
list(REMOVE_ITEM CMAKE_G "${CMAKE_CURRENT_LIST_DIR}/Anycubic.cmake")


anycubic_search_src(${CMAKE_CURRENT_LIST_DIR})
set(PLUGINS_LIST "")
foreach(cmake ${CMAKE_G})
    include(${cmake})
endforeach()



anycubic_plugins_generate_header(${PLUGINS_LIST})



