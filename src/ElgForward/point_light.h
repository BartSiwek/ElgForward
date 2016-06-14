#pragma once

#include <DirectXMath.h>

struct PointLight {
  DirectX::XMVECTOR PositionViewSpace;
  DirectX::XMVECTOR PositionWorldSpace;
  DirectX::XMVECTOR DiffuseColor;
  DirectX::XMVECTOR SpecularColor;
  float Range;
  float Intensity;
  bool Enabled;

  void Update(const DirectX::XMMATRIX& view_matrix) {
    PositionViewSpace = DirectX::XMVector4Transform(PositionWorldSpace, view_matrix);
  }
};
