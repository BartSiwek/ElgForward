#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "rendering/material.h"
#include "directx_state.h"

namespace Loaders {

bool ReadMaterial(const nlohmann::json& json_material, const filesystem::path& base_path, DirectXState* state, Rendering::Material* material);

}  // namespace Loaders
