#include "material_loader.h"

#pragma warning(push)
#pragma warning(disable: 4706)
#include <nlohmann/json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "core/json_helpers.h"
#include "rendering/typed_constant_buffer.h"
#include "rendering/materials/basic.h"
#include "rendering/dxgi_format_helper.h"
#include "loaders/texture_loader.h"
#include "shaders/registers.h"
#include "directx_state.h"

namespace Loaders {

enum class MaterialType {
  BASIC = 0,
};

bool MaterialTypeFromString(const std::string& type, MaterialType* value) {
  // TODO: Add something smarter here when there are more materials
  if (type == "basic") {
    *value = MaterialType::BASIC;
    return true;
  }
  return false;
}

bool IsTextureCompatible(const Rendering::ShaderReflection::TexureDescription& description, const TextureIdentifier& identifier) {
  if (description.BindSlotCount != Rendering::Texture::GetSlotCount(identifier.Texture)) {
    return false;
  }

  if (description.Type != Rendering::Texture::GetType(identifier.Texture)) {
    return false;
  }

  auto format_components_and_type = Rendering::DxgiFormatToComponentsAndType(Rendering::Texture::GetFormat(identifier.Texture));
  if (description.Channels != format_components_and_type.first) {
    return false;
  }

  if (description.Samples != Rendering::Texture::GetSamples(identifier.Texture)) {
    return false;
  }
 
  return true;
}

template<typename T>
void FillInTextures(const T& shader_data, const std::vector<TextureIdentifier>& textures,
                    const std::unordered_map<size_t, uint32_t>& texture_to_slot_map,
                    std::array<Rendering::Texture::Handle, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT>* material_textures) {
  auto hasher = std::hash<std::string>();

  for (const auto& mapping_entry : texture_to_slot_map) {
    auto texture_register = mapping_entry.second;
    auto texture_desc_it = std::find_if(std::begin(shader_data.ReflectionData.Texures), std::end(shader_data.ReflectionData.Texures), [texture_register](const auto& description) {
      return (description.BindSlotStart <= texture_register) && (texture_register < description.BindSlotStart + description.BindSlotCount);
    });
      
    auto name_hash = mapping_entry.first;
    auto texture_identifier_it = std::find_if(std::begin(textures), std::end(textures), [name_hash](const auto& identifier) {
      return identifier.Hash == name_hash;
    });

    if (texture_desc_it != std::end(shader_data.ReflectionData.Texures) &&
        texture_identifier_it != std::end(textures) &&
        IsTextureCompatible(*texture_desc_it, *texture_identifier_it)) {
      for (auto i = texture_desc_it->BindSlotStart; i < texture_desc_it->BindSlotStart + texture_desc_it->BindSlotCount; ++i) {
        material_textures->at(i) = texture_identifier_it->Texture;
      }
    }
  }
}

template<typename T>
bool CreateMaterial(const std::string& id, const filesystem::path& vs_path, const filesystem::path& ps_path, T* data,
                    const std::unordered_map<size_t, uint32_t>& vs_texture_to_slot_map,
                    const std::unordered_map<size_t, uint32_t>& ps_texture_to_slot_map,
                    const std::vector<TextureIdentifier>& textures, ID3D11Device* device, MaterialIdentifier* material) {
  material->Hash = std::hash<std::string>()(id);

  material->Material.VertexShader = Rendering::VertexShader::Create(vs_path, std::unordered_map<std::string, Rendering::VertexDataChannel>(), device);
  if (!material->Material.VertexShader.IsValid()) {
    return false;
  }

  auto vs_shader_data = Rendering::VertexShader::Retreive(material->Material.VertexShader);
  FillInTextures(*vs_shader_data, textures, vs_texture_to_slot_map, &material->Material.VertexShaderTextures);

  material->Material.PixelShader = Rendering::PixelShader::Create(ps_path, device);
  if (!material->Material.PixelShader.IsValid()) {
    return false;
  }

  auto ps_shader_data = Rendering::PixelShader::Retreive(material->Material.PixelShader);
  FillInTextures(*ps_shader_data, textures, ps_texture_to_slot_map, &material->Material.PixelShaderTextures);

  material->Material.Data.Reset(data);

  const auto& t_info = typeid(T);
  material->Material.TypeHash = t_info.hash_code();

  return true;
}

bool ReadBasicMaterial(const nlohmann::json& json_material, const filesystem::path& base_path,
                       const std::vector<TextureIdentifier>& textures, ID3D11Device* device,
                       MaterialIdentifier* material) {
  const std::string& name = json_material["name"];

  auto vs_path = base_path / "basic_vs.cso";
  auto ps_path = base_path / "basic_ps.cso";

  Rendering::Materials::Basic basic_material;
  std::unordered_map<size_t, uint32_t> vs_texture_to_slot_map;
  std::unordered_map<size_t, uint32_t> ps_texture_to_slot_map;
  
  float diffuse[4] = { 0.5, 0.5, 0.5, 1.0 };
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

  std::string diffuse_texture = json_material.value("diffuse_texture", "");
  if (!diffuse_texture.empty()) {
    basic_material.HasDiffuseTexture = true;
    ps_texture_to_slot_map[std::hash<std::string>()(diffuse_texture)] = DIFFUSE_TEXTURE_REGISTER;
  }

  return CreateMaterial(name, vs_path, ps_path, &basic_material, vs_texture_to_slot_map, ps_texture_to_slot_map,
                        textures, device, material);
}

bool ReadMaterial(const nlohmann::json& json_material, const filesystem::path& base_path,
                  const std::vector<TextureIdentifier>& textures, ID3D11Device* device,
                  MaterialIdentifier* material) {
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
      return ReadBasicMaterial(json_material, base_path, textures, device, material);
    default:
      DXFW_TRACE(__FILE__, __LINE__, false, "Unknown material type %S", type.c_str());
      return true;
  }

}

}  // namespace Loaders
