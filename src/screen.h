#pragma once

#include <DirectXMath.h>

inline DirectX::XMFLOAT2 GetNormalizedScreenCoordinates(float width, float height, float x, float y) {
  return DirectX::XMFLOAT2((2 * x) / width - 1.0f, 1.0f - (2 * y) / height);

}
