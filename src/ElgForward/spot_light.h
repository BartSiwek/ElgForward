#pragma once

#include <DirectXMath.h>

struct SpotLight {
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

  void Update(const DirectX::XMMATRIX& view_matrix) {
    PositionViewSpace = DirectX::XMVector4Transform(PositionWorldSpace, view_matrix);
    DirectionViewSpace = DirectX::XMVector4Transform(DirectionWorldSpace, view_matrix);
  }
};
