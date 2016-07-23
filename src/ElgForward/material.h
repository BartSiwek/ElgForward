#pragma once

#include "rendering/vertex_shader.h"
#include "rendering/pixel_shader.h"

struct Material {
  size_t Hash;
  Rendering::VertexShader::Handle VertexShader;
  Rendering::PixelShader::Handle PixelShader;
};

inline bool CreateMaterial(const std::string& id, const filesystem::path& vs_path, const filesystem::path& ps_path, ID3D11Device* device, Material* material) {
  material->Hash = std::hash<std::string>()(id);

  material->VertexShader = Rendering::VertexShader::Create(vs_path, std::unordered_map<std::string, VertexDataChannel>(), device);
  if (!material->VertexShader.IsValid()) {
    return false;
  }

  material->PixelShader = Rendering::PixelShader::Create(ps_path, device);
  if (!material->PixelShader.IsValid()) {
    return false;
  }

  return true;
}
