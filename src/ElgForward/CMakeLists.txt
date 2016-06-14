cmake_minimum_required(VERSION 3.4)

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

set(TARGET_NAME ElgForward)

set(TARGET_INCLUDE_DIR "${ROOT_DIR}/src/ElgForward")
set(TARGET_SOURCE_DIR "${ROOT_DIR}/src/ElgForward")
set(TARGET_ASSETS_DIR "${ROOT_DIR}/assets")

# Sources
set(TARGET_SOURCES
  ${TARGET_SOURCE_DIR}/camera_script.h
  ${TARGET_SOURCE_DIR}/chaiscript_helpers.cpp
  ${TARGET_SOURCE_DIR}/chaiscript_helpers.h
  ${TARGET_SOURCE_DIR}/com_helpers.h
  ${TARGET_SOURCE_DIR}/constant_buffer.h
  ${TARGET_SOURCE_DIR}/directional_light.h
  ${TARGET_SOURCE_DIR}/directx_state.h
  ${TARGET_SOURCE_DIR}/drawable.cpp
  ${TARGET_SOURCE_DIR}/drawable.h
  ${TARGET_SOURCE_DIR}/dxfw_helpers.cpp
  ${TARGET_SOURCE_DIR}/dxfw_helpers.h
  ${TARGET_SOURCE_DIR}/dxfw_wrapper.cpp
  ${TARGET_SOURCE_DIR}/dxfw_wrapper.h
  ${TARGET_SOURCE_DIR}/filesystem.cpp
  ${TARGET_SOURCE_DIR}/filesystem.h
  ${TARGET_SOURCE_DIR}/handle.h
  ${TARGET_SOURCE_DIR}/handle_cache.h
  ${TARGET_SOURCE_DIR}/hash.h
  ${TARGET_SOURCE_DIR}/index_buffer.cpp
  ${TARGET_SOURCE_DIR}/index_buffer.h
  ${TARGET_SOURCE_DIR}/main.cpp
  ${TARGET_SOURCE_DIR}/material.h
  ${TARGET_SOURCE_DIR}/mesh.cpp
  ${TARGET_SOURCE_DIR}/mesh.h
  ${TARGET_SOURCE_DIR}/perspective_lens.cpp
  ${TARGET_SOURCE_DIR}/perspective_lens.h
  ${TARGET_SOURCE_DIR}/point_light.h
  ${TARGET_SOURCE_DIR}/resource_array.h
  ${TARGET_SOURCE_DIR}/scene.h
  ${TARGET_SOURCE_DIR}/scene_loader.cpp
  ${TARGET_SOURCE_DIR}/scene_loader.h
  ${TARGET_SOURCE_DIR}/screen.h
  ${TARGET_SOURCE_DIR}/shader.cpp
  ${TARGET_SOURCE_DIR}/shader.h
  ${TARGET_SOURCE_DIR}/spot_light.h
  ${TARGET_SOURCE_DIR}/trackball_camera.cpp
  ${TARGET_SOURCE_DIR}/trackball_camera.h
  ${TARGET_SOURCE_DIR}/vertex_buffer.cpp
  ${TARGET_SOURCE_DIR}/vertex_buffer.h
  ${TARGET_SOURCE_DIR}/vertex_data.cpp
  ${TARGET_SOURCE_DIR}/vertex_data.h
  ${TARGET_SOURCE_DIR}/vertex_layout.cpp
  ${TARGET_SOURCE_DIR}/vertex_layout.h
)

set(TARGET_ASSETS_SHADERS
  ${TARGET_ASSETS_DIR}/shaders/vs.hlsl
  ${TARGET_ASSETS_DIR}/shaders/ps.hlsl
)
source_group(Shaders FILES ${TARGET_ASSETS_SHADERS})

set_source_files_properties(${TARGET_ASSETS_DIR}/shaders/vs.hlsl PROPERTIES VS_SHADER_TYPE Vertex VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT main)
set_source_files_properties(${TARGET_ASSETS_DIR}/shaders/ps.hlsl PROPERTIES VS_SHADER_TYPE Pixel VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT main)

source_group(Sources FILES ${TARGET_SOURCES})

include_directories("${TARGET_INCLUDE_DIR}")

add_executable(ElgForward "${TARGET_SOURCES}" "${TARGET_ASSETS_SHADERS}")

target_link_libraries(${TARGET_NAME} libdxfw libassimp libjson libchaiscript d3d11.lib D3DCompiler.lib dxguid.lib Shlwapi.lib)

set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_DEFINITIONS "_UNICODE;UNICODE;NOMINMAX")
set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/subsystem:windows /ENTRY:mainCRTStartup")
