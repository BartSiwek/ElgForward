#include "material_loader.h"

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "rendering/material.h"
#include "directx_state.h"

namespace Loaders {

enum class MaterialType {
  BASIC = 0,
};

bool MaterialTypeFromString(const std::string& /* type */, MaterialType* value) {
  // TODO: Add something here when there are more materials
  *value = MaterialType::BASIC;
  return true;
}

bool ReadBasicMaterial(const nlohmann::json& json_material, const filesystem::path& base_path, DirectXState* state, Rendering::Material* material) {
  const std::string& name = json_material["name"];

  auto vs_path = base_path / "basic_vs.cso";
  auto ps_path = base_path / "basic_ps.cso";

  return CreateMaterial(name, vs_path, ps_path, state->device.Get(), material);
}

bool ReadMaterial(const nlohmann::json& json_material, const filesystem::path& base_path, DirectXState* state, Rendering::Material* material) {
  bool is_valid_material_entry = json_material["name"].is_string()
                              && json_material["type"].is_string();

  if (!is_valid_material_entry) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid material entry %S", json_material.dump().c_str());
    return false;
  }

  const std::string& type = json_material["type"];

  MaterialType material_type;
  bool material_type_ok = MaterialTypeFromString(type, &material_type);
  if (!material_type_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Unknown material type %S", type.c_str());
    return false;
  }

  switch (material_type) {
    case MaterialType::BASIC:
      return ReadBasicMaterial(json_material, base_path, state, material);
    default:
      DXFW_TRACE(__FILE__, __LINE__, false, "Unknown material type %S", type.c_str());
      return true;
  }

}

}  // namespace Loaders
