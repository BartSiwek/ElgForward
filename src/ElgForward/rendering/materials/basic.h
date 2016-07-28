#pragma once

#include <DirectXMath.h>

#include "core\memory_helpers.h"

namespace Rendering {
namespace Materials {

struct BasicMaterial {
  DirectX::XMVECTOR GlobalAmbient;
  DirectX::XMVECTOR AmbientColor;
  DirectX::XMVECTOR EmissiveColor;
  DirectX::XMVECTOR DiffuseColor;
  DirectX::XMVECTOR SpecularColor;
  DirectX::XMVECTOR Reflectance;
  float Opacity;
  float SpecularPower;
  float IndexOfRefraction;
  bool HasAmbientTexture;
  bool HasEmissiveTexture;
  bool HasDiffuseTexture;
  bool HasSpecularTexture;
  bool HasSpecularPowerTexture;
  bool HasNormalTexture;
  bool HasBumpTexture;
  bool    HasOpacityTexture;
  float   BumpIntensity;
  float   SpecularScale;
  float   AlphaThreshold;
  PAD(8);
};

}  // Materials Lights
}  // namespace Rendering
