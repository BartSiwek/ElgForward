#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "directx_state.h"
#include "material.h"

namespace Loaders {

bool ReadMaterial(const nlohmann::json& json_material, const filesystem::path& base_path, DirectXState* state, Material* material);

}  // namespace Loaders
