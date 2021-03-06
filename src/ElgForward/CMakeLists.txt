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
include("${CMAKE_EXTRAS}/stb.cmake")

set(TARGET_NAME ElgForward)

set(TARGET_INCLUDE_DIR "${ROOT_DIR}/src/ElgForward")
set(TARGET_SOURCE_DIR "${ROOT_DIR}/src/ElgForward")
set(TARGET_ASSETS_DIR "${ROOT_DIR}/assets")

# Sources
set(TARGET_SOURCES_CORE
  ${TARGET_SOURCE_DIR}/core/assert.h
  ${TARGET_SOURCE_DIR}/core/buffer.h
  ${TARGET_SOURCE_DIR}/core/chaiscript_helpers.cpp
  ${TARGET_SOURCE_DIR}/core/chaiscript_helpers.h
  ${TARGET_SOURCE_DIR}/core/com_array.h
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
  ${TARGET_SOURCE_DIR}/loaders/mesh_loader.cpp
  ${TARGET_SOURCE_DIR}/loaders/mesh_loader.h
  ${TARGET_SOURCE_DIR}/loaders/scene_loader.cpp
  ${TARGET_SOURCE_DIR}/loaders/scene_loader.h
  ${TARGET_SOURCE_DIR}/loaders/texture_loader.cpp
  ${TARGET_SOURCE_DIR}/loaders/texture_loader.h
  ${TARGET_SOURCE_DIR}/loaders/transform_loader.cpp
  ${TARGET_SOURCE_DIR}/loaders/transform_loader.h
)
source_group(Sources\\Loaders FILES ${TARGET_SOURCES_LOADERS})

set(TARGET_SOURCES_RENDERING
  ${TARGET_SOURCE_DIR}/rendering/camera_script.h
  ${TARGET_SOURCE_DIR}/rendering/constant_buffer.cpp
  ${TARGET_SOURCE_DIR}/rendering/constant_buffer.h
  ${TARGET_SOURCE_DIR}/rendering/drawable.cpp
  ${TARGET_SOURCE_DIR}/rendering/drawable.h
  ${TARGET_SOURCE_DIR}/rendering/dxgi_format_helper.cpp
  ${TARGET_SOURCE_DIR}/rendering/dxgi_format_helper.h
  ${TARGET_SOURCE_DIR}/rendering/index_buffer.cpp
  ${TARGET_SOURCE_DIR}/rendering/index_buffer.h
  ${TARGET_SOURCE_DIR}/rendering/material.h
  ${TARGET_SOURCE_DIR}/rendering/mesh.cpp
  ${TARGET_SOURCE_DIR}/rendering/mesh.h
  ${TARGET_SOURCE_DIR}/rendering/pixel_shader.cpp
  ${TARGET_SOURCE_DIR}/rendering/pixel_shader.h
  ${TARGET_SOURCE_DIR}/rendering/screen.h
  ${TARGET_SOURCE_DIR}/rendering/shader_reflection.cpp
  ${TARGET_SOURCE_DIR}/rendering/shader_reflection.h
  ${TARGET_SOURCE_DIR}/rendering/structured_buffer.cpp
  ${TARGET_SOURCE_DIR}/rendering/structured_buffer.h
  ${TARGET_SOURCE_DIR}/rendering/texture.cpp
  ${TARGET_SOURCE_DIR}/rendering/texture.h
  ${TARGET_SOURCE_DIR}/rendering/transform.h
  ${TARGET_SOURCE_DIR}/rendering/transform_and_inverse_transpose.h
  ${TARGET_SOURCE_DIR}/rendering/typed_constant_buffer.h
  ${TARGET_SOURCE_DIR}/rendering/typed_structured_buffer.h
  ${TARGET_SOURCE_DIR}/rendering/vertex_buffer.cpp
  ${TARGET_SOURCE_DIR}/rendering/vertex_buffer.h
  ${TARGET_SOURCE_DIR}/rendering/vertex_data.h
  ${TARGET_SOURCE_DIR}/rendering/vertex_layout.cpp
  ${TARGET_SOURCE_DIR}/rendering/vertex_layout.h
  ${TARGET_SOURCE_DIR}/rendering/vertex_shader.cpp
  ${TARGET_SOURCE_DIR}/rendering/vertex_shader.h
)
source_group(Sources\\Rendering FILES ${TARGET_SOURCES_RENDERING})

set(TARGET_SOURCES_RENDERING_LIGHTS
  ${TARGET_SOURCE_DIR}/rendering/lights/directional_light.h
  ${TARGET_SOURCE_DIR}/rendering/lights/point_light.h
  ${TARGET_SOURCE_DIR}/rendering/lights/spot_light.h
)
source_group(Sources\\Rendering\\Lights FILES ${TARGET_SOURCES_RENDERING_LIGHTS})

set(TARGET_SOURCES_RENDERING_MATERIALS
  ${TARGET_SOURCE_DIR}/rendering/materials/basic.h
)
source_group(Sources\\Rendering\\Materials FILES ${TARGET_SOURCES_RENDERING_MATERIALS})

set(TARGET_SOURCES_RENDERING_LENS
  ${TARGET_SOURCE_DIR}/rendering/lens/perspective_lens.cpp
  ${TARGET_SOURCE_DIR}/rendering/lens/perspective_lens.h
)
source_group(Sources\\Rendering\\Lens FILES ${TARGET_SOURCES_RENDERING_LENS})

set(TARGET_SOURCES_RENDERING_CAMERAS
  ${TARGET_SOURCE_DIR}/rendering/cameras/trackball_camera.cpp
  ${TARGET_SOURCE_DIR}/rendering/cameras/trackball_camera.h
)
source_group(Sources\\Rendering\\Cameras FILES ${TARGET_SOURCES_RENDERING_CAMERAS})

set(TARGET_SOURCES
  ${TARGET_SOURCE_DIR}/directx_state.h
  ${TARGET_SOURCE_DIR}/main.cpp
  ${TARGET_SOURCE_DIR}/scene.h
)
source_group(Sources FILES ${TARGET_SOURCES})

set(TARGET_SHADERS
  ${TARGET_SOURCE_DIR}/shaders/basic.h
  ${TARGET_SOURCE_DIR}/shaders/basic_vs.hlsl
  ${TARGET_SOURCE_DIR}/shaders/basic_ps.hlsl
  ${TARGET_SOURCE_DIR}/shaders/registers.h
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
  ${TARGET_SOURCES_RENDERING_MATERIALS}
  ${TARGET_SOURCES_RENDERING_LENS}
  ${TARGET_SOURCES_RENDERING_CAMERAS}
  ${TARGET_SOURCES}
  ${TARGET_SHADERS}
)

include_directories("${TARGET_INCLUDE_DIR}")

add_executable(ElgForward "${TARGET_SOURCES_ALL}")

target_link_libraries(${TARGET_NAME} libdxfw libassimp libjson libchaiscript d3d11.lib D3DCompiler.lib dxguid.lib Shlwapi.lib)

set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_DEFINITIONS "_UNICODE;UNICODE;NOMINMAX")
set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/subsystem:windows /ENTRY:mainCRTStartup")

add_custom_command(TARGET ElgForward POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:libassimp> $<TARGET_FILE_DIR:ElgForward>)
