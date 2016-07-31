#pragma once

#include <DirectXMath.h>

#include "core\memory_helpers.h"

namespace Rendering {
namespace Materials {

struct Basic {
  DirectX::XMVECTOR DiffuseColor = { 0.5f, 0.5f, 0.5f, 1.0f };
  DirectX::XMVECTOR SpecularColor = { 0.0f, 0.0f, 0.0f, 1.0f };
  float SpecularPower = 10.0f;
};

}  // namespace Materials
}  // namespace Rendering
