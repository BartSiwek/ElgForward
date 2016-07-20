#include "light_loader.h"

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include <dxfw/dxfw.h>

#include "json_helpers.h"
#include "filesystem.h"
#include "structured_buffer.h"
#include "directional_light.h"
#include "point_light.h"
#include "spot_light.h"

bool ReadDirectionalLight(const nlohmann::json& json_light, StructuredBuffer::Handle directional_lights) {
  DirectionalLight directional_light;

  float direction[3];
  if (!ReadFloat3(json_light["direction"], direction)) {
    return false;
  }
  directional_light.DirectionWorldSpace = DirectX::XMVectorSet(direction[0], direction[1], direction[2], 0.0f);

  float diffuse[4];
  if (ReadFloat4(json_light["diffuse"], diffuse)) {
    directional_light.DiffuseColor = DirectX::XMVectorSet(diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
  }

  float specular[4];
  if (ReadFloat4(json_light["specular"], specular)) {
    directional_light.SpecularColor = DirectX::XMVectorSet(specular[0], specular[1], specular[2], specular[3]);
  }

  float intensity;
  if (ReadFloat(json_light["intensity"], &intensity)) {
    directional_light.Intensity = intensity;
  }

  bool enabled;
  if (ReadBool(json_light["enabled"], &enabled)) {
    directional_light.Enabled = enabled;
  }

  return StructuredBuffer::Add(directional_lights, &directional_light);
}

bool ReadSpotLight(const nlohmann::json& json_light, StructuredBuffer::Handle spot_lights) {
  SpotLight spot_light;

  float position[3];
  if (!ReadFloat3(json_light["position"], position)) {
    return false;
  }
  spot_light.PositionWorldSpace = DirectX::XMVectorSet(position[0], position[1], position[2], 1.0f);

  float direction[3];
  if (!ReadFloat3(json_light["direction"], direction)) {
    return false;
  }
  spot_light.DirectionWorldSpace = DirectX::XMVectorSet(direction[0], direction[1], direction[2], 0.0f);

  float diffuse[4];
  if (ReadFloat4(json_light["diffuse"], diffuse)) {
    spot_light.DiffuseColor = DirectX::XMVectorSet(diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
  }

  float specular[4];
  if (ReadFloat4(json_light["specular"], specular)) {
    spot_light.SpecularColor = DirectX::XMVectorSet(specular[0], specular[1], specular[2], specular[3]);
  }

  float angle;
  if (ReadFloat(json_light["angle"], &angle)) {
    spot_light.SpotlightAngle = angle;
  }

  float range;
  if (ReadFloat(json_light["range"], &range)) {
    spot_light.Range = range;
  }

  float intensity;
  if (ReadFloat(json_light["intensity"], &intensity)) {
    spot_light.Intensity = intensity;
  }

  bool enabled;
  if (ReadBool(json_light["enabled"], &enabled)) {
    spot_light.Enabled = enabled;
  }

  return StructuredBuffer::Add(spot_lights, &spot_light);
}

bool ReadPointLight(const nlohmann::json& json_light, StructuredBuffer::Handle point_lights) {
  PointLight point_light;

  float position[3];
  if (!ReadFloat3(json_light["position"], position)) {
    return false;
  }
  point_light.PositionWorldSpace = DirectX::XMVectorSet(position[0], position[1], position[2], 1.0f);

  float diffuse[4];
  if (ReadFloat4(json_light["diffuse"], diffuse)) {
    point_light.DiffuseColor = DirectX::XMVectorSet(diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
  }
  
  float specular[4];
  if (ReadFloat4(json_light["specular"], specular)) {
    point_light.SpecularColor = DirectX::XMVectorSet(specular[0], specular[1], specular[2], specular[3]);
  }

  float range;
  if (ReadFloat(json_light["range"], &range)) {
    point_light.Range = range;
  }

  float intensity;
  if (ReadFloat(json_light["intensity"], &intensity)) {
    point_light.Intensity = intensity;
  }

  bool enabled;
  if (ReadBool(json_light["enabled"], &enabled)) {
    point_light.Enabled = enabled;
  }

  return StructuredBuffer::Add(point_lights, &point_light);
}

bool ReadLightsFromJson(const nlohmann::json& json_lights,
                        StructuredBuffer::Handle directional_lights,
                        StructuredBuffer::Handle spot_lights,
                        StructuredBuffer::Handle point_lights) {
  const auto& json_lights_array = json_lights["lights"];
  if (!json_lights_array.is_array()) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid lights JSON %S", json_lights.dump().c_str());
    return false;
  }

  for (const auto& json_light : json_lights_array) {
    if (!json_light.is_object()) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Invalid lights entry %S", json_light.dump().c_str());
      continue;
    }

    const auto& json_light_type = json_light["type"];
    if (!json_light_type.is_string()) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Invalid light type %S", json_light_type.dump().c_str());
      continue;
    }

    const std::string& light_type = json_light_type;
    if (light_type == "directional") {
      if (!ReadDirectionalLight(json_light, directional_lights)) {
        DXFW_TRACE(__FILE__, __LINE__, false, "Reached max directional lights");
      }
      continue;
    }
    
    if (light_type == "spot") {
      if (!ReadSpotLight(json_light, spot_lights)) {
        DXFW_TRACE(__FILE__, __LINE__, false, "Reached max spot lights");
      }
      continue;
    }
    
    if (light_type == "point") {
      if (!ReadPointLight(json_light, point_lights)) {
        DXFW_TRACE(__FILE__, __LINE__, false, "Reached max point lights");
      }
      continue;
    }

    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid light type %S", light_type.c_str());
  }

  return true;
}

bool ReadLightsFromFile(const filesystem::path& lights_path,
                        StructuredBuffer::Handle directional_lights,
                        StructuredBuffer::Handle spot_lights,
                        StructuredBuffer::Handle point_lights) {
  nlohmann::json json_lights;

  bool load_ok = ReadJsonFile(lights_path, &json_lights);
  if (!load_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error reading lights file from %s", lights_path.string());
    return false;
  }

  return ReadLightsFromJson(json_lights, directional_lights, spot_lights, point_lights);
}
