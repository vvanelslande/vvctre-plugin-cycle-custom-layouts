cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0003 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(vvctre-plugin-cycle-custom-layouts)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/$<CONFIG>)

add_library(whereami STATIC
    whereami/src/whereami.c
    whereami/src/whereami.h
)
target_include_directories(whereami PUBLIC ./whereami/src)

set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
set(JSON_ImplicitConversions OFF CACHE BOOL "" FORCE)
add_subdirectory(json EXCLUDE_FROM_ALL)

add_library(vvctre-plugin-cycle-custom-layouts SHARED plugin.cpp string_util.cpp string_util.h)
target_link_libraries(vvctre-plugin-cycle-custom-layouts PRIVATE whereami nlohmann_json)

set_target_properties(vvctre-plugin-cycle-custom-layouts PROPERTIES PREFIX "" OUTPUT_NAME cycle-custom-layouts)
