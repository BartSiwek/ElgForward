cmake_minimum_required(VERSION 3.4)

set(TARGET_ASSETS_DIR "${ROOT_DIR}/assets")

# Materials
set(TARGET_ASSETS_MATERIALS
  ${TARGET_ASSETS_DIR}/materials/default.mtl
)

set_source_files_properties(${TARGET_ASSETS_DIR}/materials/default.mtl PROPERTIES HEADER_FILE_ONLY TRUE)

source_group(Assets\\Materials FILES ${TARGET_ASSETS_MATERIALS})

# Meshes
set(TARGET_ASSETS_MESHES
  ${TARGET_ASSETS_DIR}/meshes/cube.obj
  ${TARGET_ASSETS_DIR}/meshes/teapot.obj
)

set_source_files_properties(${TARGET_ASSETS_DIR}/meshes/cube.obj PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(${TARGET_ASSETS_DIR}/meshes/teapot.obj PROPERTIES HEADER_FILE_ONLY TRUE)

source_group(Assets\\Meshes FILES ${TARGET_ASSETS_MESHES})

# Scenes
set(TARGET_ASSETS_SCENES
  ${TARGET_ASSETS_DIR}/scenes/cube.json
)

set_source_files_properties(${TARGET_ASSETS_DIR}/scenes/cube.json PROPERTIES HEADER_FILE_ONLY TRUE)

source_group(Assets\\Scenes FILES ${TARGET_ASSETS_SCENES})

# Sctipts
set(TARGET_ASSETS_SCRIPTS
  ${TARGET_ASSETS_DIR}/scripts/camera.chai
)

set_source_files_properties(${TARGET_ASSETS_DIR}/scripts/camera.chai PROPERTIES HEADER_FILE_ONLY TRUE)

source_group(Assets\\Sctipts FILES ${TARGET_ASSETS_SCRIPTS})

# Textures
set(TARGET_ASSETS_TEXTURES
  ${TARGET_ASSETS_DIR}/textures/default.png
)

set_source_files_properties(${TARGET_ASSETS_DIR}/textures/default.png PROPERTIES HEADER_FILE_ONLY TRUE)

source_group(Assets\\Textures FILES ${TARGET_ASSETS_TEXTURES})

# Shaders
set(TARGET_ASSETS_SHADERS
  ${TARGET_ASSETS_DIR}/shaders/vs.hlsl
  ${TARGET_ASSETS_DIR}/shaders/ps.hlsl
)
source_group(Assets\\Shaders FILES ${TARGET_ASSETS_SHADERS})

set_source_files_properties(${TARGET_ASSETS_DIR}/shaders/vs.hlsl PROPERTIES VS_SHADER_TYPE Vertex VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT main)
set_source_files_properties(${TARGET_ASSETS_DIR}/shaders/ps.hlsl PROPERTIES VS_SHADER_TYPE Pixel VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT main)

# All assets
set(TARGET_BUILT_ASSETS
  "${TARGET_ASSETS_SHADERS}"
)

set(TARGET_COPY_ASSETS
  "${TARGET_ASSETS_MATERIALS}"
  "${TARGET_ASSETS_MESHES}"
  "${TARGET_ASSETS_SCENES}"
  "${TARGET_ASSETS_SCRIPTS}"
  "${TARGET_ASSETS_TEXTURES}"
)

set(TARGET_ASSETS
  "${TARGET_BUILT_ASSETS}"
  "${TARGET_COPY_ASSETS}"
)

# Setup the target
set(ASSETS_OUTPUTS "")
get_filename_component(TARGET_ASSETS_PARENT_DIR ${TARGET_ASSETS_DIR} DIRECTORY)
foreach(CURRENT_ASSET ${TARGET_COPY_ASSETS})
  file(RELATIVE_PATH CURRENT_ASSET_PATH ${TARGET_ASSETS_PARENT_DIR} ${CURRENT_ASSET})
  set(CURRENT_ASSET_OUTPUT_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${CURRENT_ASSET_PATH}")
  add_custom_command(DEPENDS "${CURRENT_ASSET}"
                     OUTPUT "${CURRENT_ASSET_OUTPUT_PATH}"
                     COMMAND "${CMAKE_COMMAND}"
                     ARGS -E copy_if_different "${CURRENT_ASSET}" "${CURRENT_ASSET_OUTPUT_PATH}")
  list(APPEND ASSETS_OUTPUTS ${CURRENT_ASSET_OUTPUT_PATH})
endforeach()

add_custom_target(Assets ALL DEPENDS "${ASSETS_OUTPUTS}" SOURCES "${TARGET_ASSETS}" VERBATIM)
