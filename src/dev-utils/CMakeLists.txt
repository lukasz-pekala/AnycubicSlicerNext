
option(SLIC3R_ENC_CHECK "Verify encoding of source files" 1)

if (IS_CROSS_COMPILE)
    # Force disable due to cross compilation. This fact is already printed on cli for users
    set(SLIC3R_ENC_CHECK OFF CACHE BOOL "" FORCE)
endif ()

if (SLIC3R_ENC_CHECK)
    add_executable(encoding-check encoding-check.cpp)

    # A global no-op target which depends on all encodings checks,
    # and on which in turn all checked targets depend.
    # This is done to make encoding checks the first thing to be
    # performed before actually compiling any sources of the checked targets
    # to make the check fail as early as possible.
    add_custom_target(global-encoding-check
        ALL
        DEPENDS encoding-check
    )
endif()

# Function that adds source file encoding check to a target
# using the above encoding-check binary

function(encoding_check TARGET)
    if (SLIC3R_ENC_CHECK)
        # Obtain target source files
        get_target_property(T_SOURCES ${TARGET} SOURCES)

        # Create a custom target for parallel execution
        add_custom_target(encoding-check-${TARGET}
            ALL
            DEPENDS encoding-check
            COMMENT "Checking source files encodings for target ${TARGET} (parallel)"
        )

        # Create individual custom commands for each file
        # These can be executed in parallel by make -j
        set(CHECK_COMMANDS "")
        foreach(file ${T_SOURCES})
            # Create a unique target name for each file check
            string(MAKE_C_IDENTIFIER "encoding-check-${TARGET}-${file}" FILE_CHECK_TARGET)
            
            add_custom_command(
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FILE_CHECK_TARGET}.stamp"
                COMMAND $<TARGET_FILE:encoding-check> ${TARGET} ${file}
                COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_CURRENT_BINARY_DIR}/${FILE_CHECK_TARGET}.stamp"
                DEPENDS encoding-check ${file}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Checking encoding: ${file}"
            )
            
            list(APPEND CHECK_COMMANDS "${CMAKE_CURRENT_BINARY_DIR}/${FILE_CHECK_TARGET}.stamp")
        endforeach()

        # Make the main target depend on all individual checks
        add_custom_target(encoding-check-${TARGET}-deps
            DEPENDS ${CHECK_COMMANDS}
        )
        
        add_dependencies(encoding-check-${TARGET} encoding-check-${TARGET}-deps)
        add_dependencies(${TARGET} encoding-check-${TARGET})
    endif()
endfunction()
