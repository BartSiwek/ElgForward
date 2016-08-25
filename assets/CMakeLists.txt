cmake_minimum_required(VERSION 3.4)

set(TARGET_ASSETS_DIR "${ROOT_DIR}/assets")

# Lights
set(TARGET_ASSETS_LIGHTS
  ${TARGET_ASSETS_DIR}/lights/cube_lights.json
)

source_group(Assets\\Lights FILES ${TARGET_ASSETS_LIGHTS})

# Materials
set(TARGET_ASSETS_MATERIALS
  ${TARGET_ASSETS_DIR}/materials/default.mtl
)

source_group(Assets\\Materials FILES ${TARGET_ASSETS_MATERIALS})

# Meshes
set(TARGET_ASSETS_MESHES
  ${TARGET_ASSETS_DIR}/meshes/cube.obj
  ${TARGET_ASSETS_DIR}/meshes/teapot.obj
)

source_group(Assets\\Meshes FILES ${TARGET_ASSETS_MESHES})

# Scenes
set(TARGET_ASSETS_SCENES
  ${TARGET_ASSETS_DIR}/scenes/cube.json
)

source_group(Assets\\Scenes FILES ${TARGET_ASSETS_SCENES})

# Sctipts
set(TARGET_ASSETS_SCRIPTS
  ${TARGET_ASSETS_DIR}/scripts/camera.chai
)

source_group(Assets\\Sctipts FILES ${TARGET_ASSETS_SCRIPTS})

# Textures
set(TARGET_ASSETS_TEXTURES
  ${TARGET_ASSETS_DIR}/textures/default.png
  ${TARGET_ASSETS_DIR}/textures/default_mip/default_0.png
  ${TARGET_ASSETS_DIR}/textures/default_mip/default_1.png
  ${TARGET_ASSETS_DIR}/textures/default_mip/default_2.png
  ${TARGET_ASSETS_DIR}/textures/default_mip/default_3.png
  ${TARGET_ASSETS_DIR}/textures/default_mip/default_4.png
  ${TARGET_ASSETS_DIR}/textures/default_mip/default_5.png
  ${TARGET_ASSETS_DIR}/textures/default_mip/default_6.png
  ${TARGET_ASSETS_DIR}/textures/default_mip/default_7.png
)

source_group(Assets\\Textures FILES ${TARGET_ASSETS_TEXTURES})

# All assets
set(TARGET_ASSETS
  "${TARGET_ASSETS_LIGHTS}"
  "${TARGET_ASSETS_MATERIALS}"
  "${TARGET_ASSETS_MESHES}"
  "${TARGET_ASSETS_SCENES}"
  "${TARGET_ASSETS_SCRIPTS}"
  "${TARGET_ASSETS_TEXTURES}"
)

# Setup the target
set(ASSETS_OUTPUTS "")
get_filename_component(TARGET_ASSETS_PARENT_DIR ${TARGET_ASSETS_DIR} DIRECTORY)
foreach(CURRENT_ASSET ${TARGET_ASSETS})
  file(RELATIVE_PATH CURRENT_ASSET_PATH ${TARGET_ASSETS_PARENT_DIR} ${CURRENT_ASSET})
  set(CURRENT_ASSET_OUTPUT_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${CURRENT_ASSET_PATH}")
  add_custom_command(DEPENDS "${CURRENT_ASSET}"
                     OUTPUT "${CURRENT_ASSET_OUTPUT_PATH}"
                     COMMAND "${CMAKE_COMMAND}"
                     ARGS -E copy_if_different "${CURRENT_ASSET}" "${CURRENT_ASSET_OUTPUT_PATH}")
  list(APPEND ASSETS_OUTPUTS ${CURRENT_ASSET_OUTPUT_PATH})
endforeach()

add_custom_target(Assets ALL DEPENDS "${ASSETS_OUTPUTS}" SOURCES "${TARGET_ASSETS}" VERBATIM)
