#include "material_loader.h"

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "rendering/typed_constant_buffer.h"
#include "rendering/material.h"
#include "rendering/materials/basic.h"
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
  basic_material.GlobalAmbient = { 0.0f, 0.0f, 0.0f, 0.0f };
  basic_material.AmbientColor = { 0.0f, 0.0f, 0.0f, 0.0f };
  basic_material.EmissiveColor = { 0.0f, 0.0f, 0.0f, 0.0f };
  basic_material.DiffuseColor = { 0.2f, 0.4f, 0.8f, 1.0f };
  basic_material.SpecularColor = { 0.0f, 0.0f, 0.0f, 0.0f };
  basic_material.Reflectance = { 0.0f, 0.0f, 0.0f, 0.0f };
  basic_material.Opacity = 0.0f;
  basic_material.SpecularPower = 0.0f;
  basic_material.IndexOfRefraction = 0.0f;
  basic_material.HasAmbientTexture = false;
  basic_material.HasEmissiveTexture = false;
  basic_material.HasDiffuseTexture = false;
  basic_material.HasSpecularTexture = false;
  basic_material.HasSpecularPowerTexture = false;
  basic_material.HasNormalTexture = false;
  basic_material.HasBumpTexture = false;
  basic_material.HasOpacityTexture = false;
  basic_material.BumpIntensity = 0.0f;
  basic_material.SpecularScale = 0.0f;
  basic_material.AlphaThreshold = 0.0f;

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
