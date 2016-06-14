#pragma once

#include <DirectXMath.h>

struct SpotLight {
  SpotLight(float world_x, float world_y, float world_z,
            float dir_world_x, float dir_world_y, float dir_world_z,
            float diffuse_r, float diffuse_g, float diffuse_b, float diffuse_a,
            float specular_r, float specular_g, float specular_b, float specular_a,
            float angle, float range, float intensity, bool enabled)
        : SpotlightAngle(angle), Range(range), Intensity(intensity), Enabled(enabled) {
    PositionWorldSpace = DirectX::XMVectorSet(dir_world_x, dir_world_y, dir_world_z, 1.0f);
    DirectionWorldSpace = DirectX::XMVectorSet(world_x, world_y, world_z, 1.0f);
    DiffuseColor = DirectX::XMVectorSet(diffuse_r, diffuse_g, diffuse_b, diffuse_a);
    SpecularColor = DirectX::XMVectorSet(specular_r, specular_g, specular_b, specular_a);
  }

  void Update(const DirectX::XMMATRIX& view_matrix) {
    PositionViewSpace = DirectX::XMVector4Transform(PositionWorldSpace, view_matrix);
    DirectionViewSpace = DirectX::XMVector4Transform(DirectionWorldSpace, view_matrix);
  }

  DirectX::XMVECTOR PositionViewSpace;
  DirectX::XMVECTOR PositionWorldSpace;
  DirectX::XMVECTOR DirectionViewSpace;
  DirectX::XMVECTOR DirectionWorldSpace;
  DirectX::XMVECTOR DiffuseColor;
  DirectX::XMVECTOR SpecularColor;
  float SpotlightAngle;
  float Range;
  float Intensity;
  bool Enabled;
};
