
####################################################################################################
# collect code files based of the platform
####################################################################################################

macro(eg_collect_plt_sources tgt)
target_include_directories(${tgt} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/platform/${CMAKE_SYSTEM_NAME}") # platform folder
aux_source_directory("${CMAKE_CURRENT_SOURCE_DIR}/platform/${CMAKE_SYSTEM_NAME}" PLT_SOURCE) # files in the platform folder
endmacro()

####################################################################################################
# Static link std lib
####################################################################################################

macro(eg_target_link_staticly tgt scope)
target_link_options( ${tgt} ${scope}
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<PLATFORM_ID:Windows>>:-static>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-static-libstdc++>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-static-libgcc>
)

endmacro()

####################################################################################################
# Print host system info
####################################################################################################

function(eg_print_sys_info key)
    cmake_host_system_information(RESULT ${key} QUERY ${key})
    message(STATUS "${key} : ${${key}}")
endfunction()

####################################################################################################
# add options
####################################################################################################

include(FeatureSummary)

function(eg_options)
    math(EXPR remainder "${ARGC} % 3")

    if(NOT remainder EQUAL 0)
        message(FATAL_ERROR
            "eg_options() expects arguments in groups of 3:\n"
            "  <NAME> <DESCRIPTION> <DEFAULT>"
        )
    endif()

    math(EXPR last "${ARGC} - 1")

    foreach(i RANGE 0 ${last} 3)
        math(EXPR i1 "${i} + 1")
        math(EXPR i2 "${i} + 2")

        list(GET ARGV ${i}  opt)
        list(GET ARGV ${i1} desc)
        list(GET ARGV ${i2} state)

        option(${opt} "${desc}" ${state})
        add_feature_info(${opt} ${opt} "[${desc}]")
    endforeach()
endfunction()


####################################################################################################
# add defines for options
####################################################################################################

function(eg_options_defs opts)
    list(LENGTH ${opts} len)
    math(EXPR last "${len} - 1")

    foreach(i RANGE 0 ${last} 3)
        list(GET ${opts} ${i} opt)

        if(${opt})
            add_compile_definitions(${opt})
        endif()
    endforeach()
endfunction()
