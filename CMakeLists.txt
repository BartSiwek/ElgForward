cmake_minimum_required(VERSION 3.4)

project(ElgForward VERSION 0.1 LANGUAGES CXX)

include(CheckIncludeFiles)

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
include("${CMAKE_EXTRAS}/assimp.cmake")
include("${CMAKE_EXTRAS}/json.cmake")
include("${CMAKE_EXTRAS}/chaiscript.cmake")

set(PROJECT_ASSETS_DIR "${ROOT_DIR}/assets")
set(PROJECT_INCLUDE_DIR "${ROOT_DIR}/src")
set(PROJECT_SOURCE_DIR "${ROOT_DIR}/src")

file(GLOB_RECURSE PROJECT_ASSETS ${PROJECT_ASSETS_DIR}/*)

set(PROJECT_SRCS
  ${PROJECT_SOURCE_DIR}/main.cpp
  ${PROJECT_SOURCE_DIR}/shaders/hlsl_definitions.h
  ${PROJECT_SOURCE_DIR}/constant_buffer.h
  ${PROJECT_SOURCE_DIR}/com_helpers.h
  ${PROJECT_SOURCE_DIR}/directx_state.h
  ${PROJECT_SOURCE_DIR}/drawable.h
  ${PROJECT_SOURCE_DIR}/drawable.cpp
  ${PROJECT_SOURCE_DIR}/dxfw_wrapper.h
  ${PROJECT_SOURCE_DIR}/dxfw_wrapper.cpp
  ${PROJECT_SOURCE_DIR}/dxfw_helpers.h
  ${PROJECT_SOURCE_DIR}/dxfw_helpers.cpp
  ${PROJECT_SOURCE_DIR}/filesystem.h
  ${PROJECT_SOURCE_DIR}/filesystem.cpp
  ${PROJECT_SOURCE_DIR}/material.h
  ${PROJECT_SOURCE_DIR}/mesh.h
  ${PROJECT_SOURCE_DIR}/mesh.cpp
  ${PROJECT_SOURCE_DIR}/handle.h
  ${PROJECT_SOURCE_DIR}/handle_cache.h
  ${PROJECT_SOURCE_DIR}/hash.h
  ${PROJECT_SOURCE_DIR}/index_buffer.h
  ${PROJECT_SOURCE_DIR}/index_buffer.cpp
  ${PROJECT_SOURCE_DIR}/perspective_lens.h
  ${PROJECT_SOURCE_DIR}/perspective_lens.cpp
  ${PROJECT_SOURCE_DIR}/resource_array.h
  ${PROJECT_SOURCE_DIR}/scene.h
  ${PROJECT_SOURCE_DIR}/scene_loader.h
  ${PROJECT_SOURCE_DIR}/scene_loader.cpp
  ${PROJECT_SOURCE_DIR}/screen.h
  ${PROJECT_SOURCE_DIR}/shader.h
  ${PROJECT_SOURCE_DIR}/shader.cpp
  ${PROJECT_SOURCE_DIR}/trackball_camera.h
  ${PROJECT_SOURCE_DIR}/trackball_camera.cpp
  ${PROJECT_SOURCE_DIR}/vertex_buffer.h
  ${PROJECT_SOURCE_DIR}/vertex_buffer.cpp
  ${PROJECT_SOURCE_DIR}/vertex_data.h
  ${PROJECT_SOURCE_DIR}/vertex_data.cpp
  ${PROJECT_SOURCE_DIR}/vertex_layout.h
  ${PROJECT_SOURCE_DIR}/vertex_layout.cpp
)

set_source_files_properties(${PROJECT_SOURCE_DIR}/shaders/vs.hlsl PROPERTIES VS_SHADER_TYPE Vertex VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT main)
set_source_files_properties(${PROJECT_SOURCE_DIR}/shaders/ps.hlsl PROPERTIES VS_SHADER_TYPE Pixel VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT main)

set(PROJECT_SHADERS
  ${PROJECT_SOURCE_DIR}/shaders/vs.hlsl
  ${PROJECT_SOURCE_DIR}/shaders/ps.hlsl
)

include_directories("${PROJECT_INCLUDE_DIR}")

add_executable(${PROJECT_NAME} "${PROJECT_SRCS}" "${PROJECT_SHADERS}")

target_link_libraries(${PROJECT_NAME} libdxfw libassimp libjson libchaiscript d3d11.lib D3DCompiler.lib dxguid.lib Shlwapi.lib)

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "_UNICODE;UNICODE;NOMINMAX")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/subsystem:windows /ENTRY:mainCRTStartup")

get_filename_component(PROJECT_ASSETS_PARENT_DIR ${PROJECT_ASSETS_DIR} DIRECTORY)
foreach(CURRENT_ASSET ${PROJECT_ASSETS})
	file(RELATIVE_PATH CURRENT_ASSET_PATH ${PROJECT_ASSETS_PARENT_DIR} ${CURRENT_ASSET})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   	 COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CURRENT_ASSET}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${CURRENT_ASSET_PATH}")
endforeach()

get_target_property(ASSIMP_DLL_LOCATION libassimp "IMPORTED_LOCATION")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASSIMP_DLL_LOCATION}" $<TARGET_FILE_DIR:${PROJECT_NAME}>)
