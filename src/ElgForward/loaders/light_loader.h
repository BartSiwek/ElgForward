#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "rendering/structured_buffer.h"

namespace Loaders {

bool ReadLightsFromFile(const filesystem::path& lights_path,
                        Rendering::StructuredBuffer::Handle directional_lights,
                        Rendering::StructuredBuffer::Handle spot_lights,
                        Rendering::StructuredBuffer::Handle point_lights);

bool ReadLightsFromJson(const nlohmann::json& json_lights,
                        Rendering::StructuredBuffer::Handle directional_lights,
                        Rendering::StructuredBuffer::Handle spot_lights,
                        Rendering::StructuredBuffer::Handle point_lights);

}  // namespace Loaders
