#pragma once

#include <DirectXMath.h>

struct PointLight {
  PointLight(float world_x, float world_y, float world_z,
             float diffuse_r, float diffuse_g, float diffuse_b, float diffuse_a,
             float specular_r, float specular_g, float specular_b, float specular_a,
             float range, float intensity, bool enabled)
        : Range(range), Intensity(intensity), Enabled(enabled) {
    PositionWorldSpace = DirectX::XMVectorSet(world_x, world_y, world_z, 1.0f);
    DiffuseColor = DirectX::XMVectorSet(diffuse_r, diffuse_g, diffuse_b, diffuse_a);
    SpecularColor = DirectX::XMVectorSet(specular_r, specular_g, specular_b, specular_a);
  }

  void Update(const DirectX::XMMATRIX& view_matrix) {
    PositionViewSpace = DirectX::XMVector4Transform(PositionWorldSpace, view_matrix);
  }

  DirectX::XMVECTOR PositionViewSpace;
  DirectX::XMVECTOR PositionWorldSpace;
  DirectX::XMVECTOR DiffuseColor;
  DirectX::XMVECTOR SpecularColor;
  float Range;
  float Intensity;
  bool Enabled;
};
