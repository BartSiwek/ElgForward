#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "rendering/transform.h"

namespace Loaders {

bool ReadTransform(const std::string& parent_name, const nlohmann::json& json_transform, ID3D11Device* device, Rendering::Transform::Transform* transform);

}  // namespace Loaders
