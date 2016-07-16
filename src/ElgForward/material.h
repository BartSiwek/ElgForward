#pragma once

#include "shader.h"

struct Material {
  size_t Hash;
  VertexShaderHandle VertexShader;
  PixelShaderHandle PixelShader;
};

inline bool CreateMaterial(const std::string& /* id */, const std::string& /* type */, const filesystem::path& /* base_path */, ID3D11Device* /* device */, Material* /* material */) {
  // basic -> vs.cso, ps.cso

  /*
  material->Hash = std::hash<std::string>()(id);

  material->VertexShader = CreateVertexShader(vs_path, std::unordered_map<std::string, VertexDataChannel>(), device);
  if (!material->VertexShader.IsValid()) {
    return false;
  }

  material->PixelShader = CreatePixelShader(ps_path, device);
  if (!material->PixelShader.IsValid()) {
    return false;
  }
  */

  return true;
}