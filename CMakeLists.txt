cmake_minimum_required(VERSION 3.4)

project(ElgForward VERSION 0.1 LANGUAGES CXX)

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CMAKE_EXTRAS ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(MSVC)
  foreach(lang C CXX)
    if("${CMAKE_${lang}_FLAGS}" MATCHES "/W[0-4]")
      string(REGEX REPLACE "/W[1-3]" "/W4 /WX" CMAKE_${lang}_FLAGS "${CMAKE_${lang}_FLAGS}")
    else("${CMAKE_${lang}_FLAGS}" MATCHES "/W[1-3]")
      set(CMAKE_${lang}_FLAGS "${CMAKE_${lang}_FLAGS} /W4 /WX")
    endif()
  endforeach()
endif()

include("${CMAKE_EXTRAS}/dxfw.cmake")
include("${CMAKE_EXTRAS}/tinyobjloader.cmake")

set(PROJECT_ASSETS_DIR "${ROOT_DIR}/assets")
set(PROJECT_INCLUDE_DIR "${ROOT_DIR}/src")
set(PROJECT_SOURCE_DIR "${ROOT_DIR}/src")

set(PROJECT_ASSETS
  # ${PROJECT_ASSETS_DIR}/shaders.hlsl
)

set(PROJECT_SRCS
  ${PROJECT_SOURCE_DIR}/main.cpp
)

include_directories("${PROJECT_INCLUDE_DIR}")

add_executable(${PROJECT_NAME} "${PROJECT_SRCS}")

target_link_libraries(${PROJECT_NAME} libdxfw libtinyobjloader d3d11.lib D3DCompiler.lib Shlwapi.lib)

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "_UNICODE;UNICODE")
# set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/subsystem:windows /ENTRY:mainCRTStartup")

foreach(CURRENT_ASSET ${PROJECT_ASSETS})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   	   COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CURRENT_ASSET}" $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endforeach()



