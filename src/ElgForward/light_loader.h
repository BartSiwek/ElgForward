#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "filesystem.h"
#include "structured_buffer.h"

void ReadLightsFromFile(const filesystem::path& lights_path,
                        StructuredBuffer::StructuredBufferHandle directional_lights,
                        StructuredBuffer::StructuredBufferHandle spot_lights,
                        StructuredBuffer::StructuredBufferHandle point_lights);

void ReadLightsFromJson(const nlohmann::json& json_lights,
                        StructuredBuffer::StructuredBufferHandle directional_lights,
                        StructuredBuffer::StructuredBufferHandle spot_lights,
                        StructuredBuffer::StructuredBufferHandle point_lights);