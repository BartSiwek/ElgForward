#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

namespace Rendering {

inline DirectX::XMFLOAT2 GetNormalizedScreenCoordinates(float width, float height, float x, float y) {
  return DirectX::XMFLOAT2((2 * x) / width - 1.0f, 1.0f - (2 * y) / height);

}

inline void SetViewportSize(D3D11_VIEWPORT* viewport, unsigned int width, unsigned int height) {
  ZeroMemory(viewport, sizeof(D3D11_VIEWPORT));
  viewport->TopLeftX = 0.0f;
  viewport->TopLeftY = 0.0f;
  viewport->Width = static_cast<float>(width);
  viewport->Height = static_cast<float>(height);
  viewport->MinDepth = 0.0f;
  viewport->MaxDepth = 1.0f;
}

}  // namespace Rendering
