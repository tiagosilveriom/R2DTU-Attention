cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(r2dtu)

#Force out of source build
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Please do a out of source build, preferable using the task runner")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options (-fcolor-diagnostics)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options (-fdiagnostics-color)
endif()


set(DEPEND_DIR $ENV{DEPEND_DIR} CACHE STRING "")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist/modules)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist/)
set(CMAKE_COLOR_MAKEFILE ON)

include(FindPackageHandleStandardArgs)
find_package(Threads REQUIRED)


if ("${MODULE_TARGET}" STREQUAL "onboard")
    include(onboard)
elseif ("${MODULE_TARGET}" STREQUAL "field")
    include(field)
    add_subdirectory(testing/unit)
else()
    message(FATAL_ERROR "No MODULE_TARGET specified")
endif()

configure_file(${CMAKE_SOURCE_DIR}/r2dtu.toml ${CMAKE_BINARY_DIR}/dist/r2dtu.toml COPYONLY)
