cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(las-parser VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------------------------------------------------------------------
#   LAS parser
# ---------------------------------------------------------------------------

add_library(las-parser ${OS_BUNDLE})

target_sources(las-parser
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include/las_parser/las2.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/las_parser/lasCore.hpp

    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/las2.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/las2generate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/lasCore.cpp
    )

target_include_directories(mxz-az
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    )
