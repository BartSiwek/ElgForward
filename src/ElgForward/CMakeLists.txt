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
set(TARGET_SOURCES_CORE
  ${TARGET_SOURCE_DIR}/core/chaiscript_helpers.cpp
  ${TARGET_SOURCE_DIR}/core/chaiscript_helpers.h
  ${TARGET_SOURCE_DIR}/core/com_helpers.h
  ${TARGET_SOURCE_DIR}/core/filesystem.cpp
  ${TARGET_SOURCE_DIR}/core/filesystem.h
  ${TARGET_SOURCE_DIR}/core/handle.h
  ${TARGET_SOURCE_DIR}/core/handle_cache.h
  ${TARGET_SOURCE_DIR}/core/hash.h
  ${TARGET_SOURCE_DIR}/core/json_helpers.cpp
  ${TARGET_SOURCE_DIR}/core/json_helpers.h
  ${TARGET_SOURCE_DIR}/core/memory_helpers.h
  ${TARGET_SOURCE_DIR}/core/resource_array.h
)
source_group(Sources\\Core FILES ${TARGET_SOURCES_CORE})

set(TARGET_SOURCES_DXFW
  ${TARGET_SOURCE_DIR}/dxfw/dxfw_helpers.cpp
  ${TARGET_SOURCE_DIR}/dxfw/dxfw_helpers.h
  ${TARGET_SOURCE_DIR}/dxfw/dxfw_wrapper.cpp
  ${TARGET_SOURCE_DIR}/dxfw/dxfw_wrapper.h
)
source_group(Sources\\Dxfw FILES ${TARGET_SOURCES_DXFW})

set(TARGET_SOURCES_LOADERS
  ${TARGET_SOURCE_DIR}/loaders/camera_loader.cpp
  ${TARGET_SOURCE_DIR}/loaders/camera_loader.h
  ${TARGET_SOURCE_DIR}/loaders/light_loader.cpp
  ${TARGET_SOURCE_DIR}/loaders/light_loader.h
  ${TARGET_SOURCE_DIR}/loaders/material_loader.cpp
  ${TARGET_SOURCE_DIR}/loaders/material_loader.h
  ${TARGET_SOURCE_DIR}/loaders/scene_loader.cpp
  ${TARGET_SOURCE_DIR}/loaders/scene_loader.h
)
source_group(Sources\\Loaders FILES ${TARGET_SOURCES_LOADERS})

set(TARGET_SOURCES_RENDERING
  ${TARGET_SOURCE_DIR}/rendering/constant_buffer.cpp
  ${TARGET_SOURCE_DIR}/rendering/constant_buffer.h
  ${TARGET_SOURCE_DIR}/rendering/drawable.cpp
  ${TARGET_SOURCE_DIR}/rendering/drawable.h
  ${TARGET_SOURCE_DIR}/rendering/index_buffer.cpp
  ${TARGET_SOURCE_DIR}/rendering/index_buffer.h
  ${TARGET_SOURCE_DIR}/rendering/structured_buffer.cpp
  ${TARGET_SOURCE_DIR}/rendering/structured_buffer.h
  ${TARGET_SOURCE_DIR}/rendering/vertex_buffer.cpp
  ${TARGET_SOURCE_DIR}/rendering/vertex_buffer.h
  ${TARGET_SOURCE_DIR}/rendering/vertex_layout.cpp
  ${TARGET_SOURCE_DIR}/rendering/vertex_layout.h
)
source_group(Sources\\Rendering FILES ${TARGET_SOURCES_RENDERING})

set(TARGET_SOURCES_RENDERING_LIGHTS
  ${TARGET_SOURCE_DIR}/rendering/lights/directional_light.h
  ${TARGET_SOURCE_DIR}/rendering/lights/point_light.h
  ${TARGET_SOURCE_DIR}/rendering/lights/spot_light.h
)
source_group(Sources\\Rendering\\Lights FILES ${TARGET_SOURCES_RENDERING_LIGHTS})

set(TARGET_SOURCES
  ${TARGET_SOURCE_DIR}/camera_script.h
  ${TARGET_SOURCE_DIR}/directx_state.h
  ${TARGET_SOURCE_DIR}/main.cpp
  ${TARGET_SOURCE_DIR}/material.h
  ${TARGET_SOURCE_DIR}/mesh.cpp
  ${TARGET_SOURCE_DIR}/mesh.h
  ${TARGET_SOURCE_DIR}/perspective_lens.cpp
  ${TARGET_SOURCE_DIR}/perspective_lens.h
  ${TARGET_SOURCE_DIR}/scene.h
  ${TARGET_SOURCE_DIR}/screen.h
  ${TARGET_SOURCE_DIR}/shader.cpp
  ${TARGET_SOURCE_DIR}/shader.h
  ${TARGET_SOURCE_DIR}/trackball_camera.cpp
  ${TARGET_SOURCE_DIR}/trackball_camera.h
  ${TARGET_SOURCE_DIR}/vertex_data.cpp
  ${TARGET_SOURCE_DIR}/vertex_data.h
)
source_group(Sources FILES ${TARGET_SOURCES})

set(TARGET_SHADERS
  ${TARGET_SOURCE_DIR}/shaders/basic.h
  ${TARGET_SOURCE_DIR}/shaders/basic_vs.hlsl
  ${TARGET_SOURCE_DIR}/shaders/basic_ps.hlsl
)

set_source_files_properties(${TARGET_SOURCE_DIR}/shaders/basic.h PROPERTIES VS_SHADER_MODEL 5.0)
set_source_files_properties(${TARGET_SOURCE_DIR}/shaders/basic_vs.hlsl PROPERTIES VS_SHADER_TYPE Vertex VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT main)
set_source_files_properties(${TARGET_SOURCE_DIR}/shaders/basic_ps.hlsl PROPERTIES VS_SHADER_TYPE Pixel VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT main)

source_group(Shaders FILES ${TARGET_SHADERS})

set(TARGET_SOURCES_ALL
  ${TARGET_SOURCES_CORE}
  ${TARGET_SOURCES_DXFW}
  ${TARGET_SOURCES_LOADERS}
  ${TARGET_SOURCES_RENDERING}
  ${TARGET_SOURCES_RENDERING_LIGHTS}
  ${TARGET_SOURCES}
  ${TARGET_SHADERS}
)

include_directories("${TARGET_INCLUDE_DIR}")

add_executable(ElgForward "${TARGET_SOURCES_ALL}")

target_link_libraries(${TARGET_NAME} libdxfw libassimp libjson libchaiscript d3d11.lib D3DCompiler.lib dxguid.lib Shlwapi.lib)

set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_DEFINITIONS "_UNICODE;UNICODE;NOMINMAX")
set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/subsystem:windows /ENTRY:mainCRTStartup")
