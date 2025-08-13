function(anycubic_plugins_generate_header )
    set(gen_embed_file_header ${CMAKE_CURRENT_BINARY_DIR}/plugins_list.h)
    file(WRITE ${gen_embed_file_header} "#define PLUGINS_LIST (")
    foreach(item ${ARGN})
        file(APPEND ${gen_embed_file_header} "${item},")
    endforeach()
    file(APPEND ${gen_embed_file_header} "NONE)\n")
    set(PLUGINS_LIST_SIZE ${ARGC})
    file(APPEND ${gen_embed_file_header} "#define PLUGINS_LIST_SIZE ${PLUGINS_LIST_SIZE}\n")
endfunction()



anycubic_search_src(${CMAKE_CURRENT_LIST_DIR})