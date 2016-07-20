#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "filesystem.h"
#include "structured_buffer.h"

bool ReadLightsFromFile(const filesystem::path& lights_path,
                        StructuredBuffer::Handle directional_lights,
                        StructuredBuffer::Handle spot_lights,
                        StructuredBuffer::Handle point_lights);

bool ReadLightsFromJson(const nlohmann::json& json_lights,
                        StructuredBuffer::Handle directional_lights,
                        StructuredBuffer::Handle spot_lights,
                        StructuredBuffer::Handle point_lights);