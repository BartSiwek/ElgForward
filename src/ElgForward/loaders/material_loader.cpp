#include "material_loader.h"

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "core/json_helpers.h"
#include "rendering/typed_constant_buffer.h"
#include "rendering/material.h"
#include "rendering/materials/basic.h"
#include "directx_state.h"

namespace Loaders {

enum class MaterialType {
  BASIC = 0,
};

bool MaterialTypeFromString(const std::string& type, MaterialType* value) {
  // TODO: Add something here when there are more materials
  if (type == "basic") {
    *value = MaterialType::BASIC;
    return true;
  }
  return false;
}

bool CreateMaterial(const std::string& id, const filesystem::path& vs_path, const filesystem::path& ps_path, ID3D11Device* device,
                    Rendering::ConstantBuffer::Handle material_constant_buffer, Rendering::Material* material) {
  material->Hash = std::hash<std::string>()(id);

  material->VertexShader = Rendering::VertexShader::Create(vs_path, std::unordered_map<std::string, Rendering::VertexDataChannel>(), device);
  if (!material->VertexShader.IsValid()) {
    return false;
  }

  material->PixelShader = Rendering::PixelShader::Create(ps_path, device);
  if (!material->PixelShader.IsValid()) {
    return false;
  }

  material->MaterialConstantBuffer = material_constant_buffer;

  return true;
}

bool ReadBasicMaterial(const nlohmann::json& json_material, const filesystem::path& base_path, DirectXState* state, Rendering::Material* material) {
  const std::string& name = json_material["name"];

  auto vs_path = base_path / "basic_vs.cso";
  auto ps_path = base_path / "basic_ps.cso";

  Rendering::Materials::Basic basic_material;
  
  float diffuse[4];
  if (Core::ReadFloat4(json_material.value("diffuse", nlohmann::json::array({ 0.5, 0.5, 0.5, 1.0 })), diffuse)) {
    basic_material.DiffuseColor = DirectX::XMVectorSet(diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
  }

  float specular[4];
  if (Core::ReadFloat4(json_material.value("specular", nlohmann::json::array({ 0.0, 0.0, 0.0, 0.0 })), specular)) {
    basic_material.SpecularColor = DirectX::XMVectorSet(specular[0], specular[1], specular[2], specular[3]);
  }

  float specular_power;
  if (Core::ReadFloat(json_material.value("specular_power", nlohmann::json::number_float_t(10.0f)), &specular_power)) {
    basic_material.SpecularPower = specular_power;
  }

  auto material_constant_buffer = Rendering::ConstantBuffer::Create(name, &basic_material, state->device.Get());

  return CreateMaterial(name, vs_path, ps_path, state->device.Get(), static_cast<Rendering::ConstantBuffer::Handle>(material_constant_buffer), material);
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
