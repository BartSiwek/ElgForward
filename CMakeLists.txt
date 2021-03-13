cmake_minimum_required(VERSION 3.4)

project(ElgForward VERSION 0.1 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)

if (MSVC)
  add_compile_options(/bigobj)
endif()

include(CheckIncludeFiles)

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CMAKE_EXTRAS ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_subdirectory(src)
add_subdirectory(assets)
