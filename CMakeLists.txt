cmake_minimum_required(VERSION 3.30)

project(
    GameEngine
    DESCRIPTION "Cross Platform C++ Game Engine"
    LANGUAGES C CXX
    VERSION 0.0.0
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(eg_misc)
include(Options)
include(Harden)
include(Coverage)
include(ASLR_OFF)
include(CompilerFlags)

if(WIN32)
    add_definitions(-DNOMINMAX)
else()
    add_definitions(-DAPIENTRY=)
endif()

####################################################################################################
# Visibility
####################################################################################################

set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

####################################################################################################
# PGO
####################################################################################################

set(GEN_PROFILE
    $<$<CXX_COMPILER_ID:GNU>:-fprofile-generate>
    $<$<CXX_COMPILER_ID:Clang>:-fprofile-instr-generate>
    $<$<CXX_COMPILER_ID:MSVC>:/GENPROFILE>
)

set(USE_PROFILE
    $<$<CXX_COMPILER_ID:GNU>:-fprofile-use>
    $<$<CXX_COMPILER_ID:Clang>:-fprofile-instr-use>
    $<$<CXX_COMPILER_ID:MSVC>:/USEPROFILE>
)

if(PROFILING_MODE STREQUAL "GENERATE")
    add_compile_options(${GEN_PROFILE})
elseif(PROFILING_MODE STREQUAL "USE")
    add_compile_options(${USE_PROFILE})
endif()

####################################################################################################
# IPO/LTO
####################################################################################################

if(NOT EG_COVERAGE AND NOT MSVC AND NOT PROFILING_MODE)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT output)

    if(IPO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    endif()
endif()

####################################################################################################
# Deps
####################################################################################################

include(InstaledPackages)
include(Deps)

####################################################################################################
# Resources
####################################################################################################

include(CMakeRC)

file(GLOB_RECURSE RES_FILES
    RELATIVE "${CMAKE_SOURCE_DIR}"
    CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/res/*"
)

cmrc_add_resource_library( resources
    ALIAS res
    NAMESPACE res

    ${RES_FILES}
)

####################################################################################################
# Sub Dirs
####################################################################################################

add_subdirectory(Engine)
add_subdirectory(Game)

####################################################################################################
# TEST
####################################################################################################

if(EG_UNIT_TESTS)
    include(CTest)
    add_subdirectory(tests/unit)
endif()

if(EG_FUZZ_TESTS)
    include(CTest)
    add_subdirectory(tests/fuzz)
endif()

####################################################################################################
# Print Host Info
####################################################################################################

message(STATUS "---------------------------[Host Info]----------------------------------")
eg_print_sys_info(NUMBER_OF_LOGICAL_CORES)
eg_print_sys_info(NUMBER_OF_PHYSICAL_CORES)
eg_print_sys_info(HOSTNAME)
eg_print_sys_info(FQDN)
eg_print_sys_info(TOTAL_VIRTUAL_MEMORY)
eg_print_sys_info(AVAILABLE_VIRTUAL_MEMORY)
eg_print_sys_info(TOTAL_PHYSICAL_MEMORY)
eg_print_sys_info(AVAILABLE_PHYSICAL_MEMORY)
eg_print_sys_info(IS_64BIT)
eg_print_sys_info(HAS_FPU)
eg_print_sys_info(HAS_MMX)
eg_print_sys_info(HAS_MMX_PLUS)
eg_print_sys_info(HAS_SSE)
eg_print_sys_info(HAS_SSE2)
eg_print_sys_info(HAS_SSE_FP)
eg_print_sys_info(HAS_SSE_MMX)
eg_print_sys_info(HAS_AMD_3DNOW)
eg_print_sys_info(HAS_AMD_3DNOW_PLUS)
eg_print_sys_info(HAS_IA64)
eg_print_sys_info(HAS_SERIAL_NUMBER)
eg_print_sys_info(PROCESSOR_SERIAL_NUMBER)
eg_print_sys_info(PROCESSOR_NAME)
eg_print_sys_info(PROCESSOR_DESCRIPTION)
eg_print_sys_info(OS_NAME)
eg_print_sys_info(OS_RELEASE)
eg_print_sys_info(OS_VERSION)
eg_print_sys_info(OS_PLATFORM)
eg_print_sys_info(DISTRIB_INFO)
message(STATUS "--------------------------------------------------------------------------------")
