#pragma once

#include <DirectXMath.h>

#include "memory_helpers.h"

struct DirectionalLight {
  DirectionalLight()
      : Intensity(1.0f), Enabled(true) {
    DirectionWorldSpace = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    DiffuseColor = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    SpecularColor = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
  }

  DirectionalLight(float dir_world_x, float dir_world_y, float dir_world_z,
                   float diffuse_r, float diffuse_g, float diffuse_b, float diffuse_a,
                   float specular_r, float specular_g, float specular_b, float specular_a,
                   float intensity, bool enabled)
      : Intensity(intensity), Enabled(enabled) {
    DirectionWorldSpace = DirectX::XMVectorSet(dir_world_x, dir_world_y, dir_world_z, 0.0f);
    DiffuseColor = DirectX::XMVectorSet(diffuse_r, diffuse_g, diffuse_b, diffuse_a);
    SpecularColor = DirectX::XMVectorSet(specular_r, specular_g, specular_b, specular_a);
  }

  void Update(const DirectX::XMMATRIX& view_matrix) {
    DirectionViewSpace = DirectX::XMVector4Transform(DirectionWorldSpace, view_matrix);
  }

  DirectX::XMVECTOR DirectionViewSpace;
  DirectX::XMVECTOR DirectionWorldSpace;
  DirectX::XMVECTOR DiffuseColor;
  DirectX::XMVECTOR SpecularColor;
  float Intensity;
  bool Enabled;
};
