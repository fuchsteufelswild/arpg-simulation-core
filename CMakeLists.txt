cmake_minimum_required(VERSION 3.28)

project(arpg_sim
    VERSION 0.1.0
    DESCRIPTION "Deterministic ARPG simulation core"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(SIM_BUILD_TESTS "Build unit and replay tests" ON)
option(SIM_DEBUG_LOGGING "Enable diagnostic error logging in DLL" OFF)
option(SIM_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

if(MSVC)
    add_compile_options(
        /W4
        /permissive-
        /Zc:__cplusplus
        /Zc:preprocessor
        /utf-8
        /MP
        /EHsc
    )
    if(SIM_WARNINGS_AS_ERRORS)
        add_compile_options(/WX)
    endif()
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error LANGUAGES CXX)
if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    message(STATUS "IPO/LTO enabled for Release builds")
else()
    message(STATUS "IPO/LTO not supported: ${ipo_error}")
endif()

add_subdirectory(sim)
add_subdirectory(sim_api)

if(SIM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()