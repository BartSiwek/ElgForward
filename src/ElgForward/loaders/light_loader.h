#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include <nlohmann/json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "rendering/typed_structured_buffer.h"
#include "rendering/lights/directional_light.h"
#include "rendering/lights/point_light.h"
#include "rendering/lights/spot_light.h"

namespace Loaders {

bool ReadLightsFromFile(const filesystem::path& lights_path,
                        Rendering::StructuredBuffer::TypedHandle<Rendering::Lights::DirectionalLight> directional_lights,
                        Rendering::StructuredBuffer::TypedHandle<Rendering::Lights::SpotLight> spot_lights,
                        Rendering::StructuredBuffer::TypedHandle<Rendering::Lights::PointLight> point_lights);

bool ReadLightsFromJson(const nlohmann::json& json_lights,
                        Rendering::StructuredBuffer::TypedHandle<Rendering::Lights::DirectionalLight> directional_lights,
                        Rendering::StructuredBuffer::TypedHandle<Rendering::Lights::SpotLight> spot_lights,
                        Rendering::StructuredBuffer::TypedHandle<Rendering::Lights::PointLight> point_lights);

}  // namespace Loaders
