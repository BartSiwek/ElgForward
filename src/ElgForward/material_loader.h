#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "filesystem.h"
#include "directx_state.h"
#include "material.h"

bool ReadMaterial(const nlohmann::json& json_material, const filesystem::path& base_path, DirectXState* state, Material* material);
