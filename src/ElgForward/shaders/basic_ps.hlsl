#pragma pack_matrix(row_major)

#include "registers.h"
#include "basic.h"

cbuffer PerMaterialConstants : PER_MATERIAL_CONSTANT_BUFFER_REGISTER {
  float4 DiffuseColor;
  float4 SpecularColor;
  float SpecularPower;
  bool HasDiffuseTexture;
};

struct PointLight {
  float4 PositionViewSpace;
  float4 PositionWorldSpace;
  float4 DiffuseColor;
  float4 SpecularColor;
  float Range;
  float Intensity;
  bool Enabled;
  float pad;
};

StructuredBuffer <PointLight> PointLights : POINT_LIGHT_BUFFER_REGISTER;

Texture2D<float4> DiffuseTexture : DIFFUSE_TEXTURE_REGISTER;
SamplerState ExperimentalSampler;

float4 main(VertexShaderOutput input) : SV_TARGET {
  float3 n = normalize(input.Normal);

  float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
  for (int i = 0; i < PointLightCount; ++i) {
    float3 lu = (PointLights[i].PositionViewSpace - input.PositionViewSpace).xyz;
    float3 l = normalize(lu);

    float nDotL = dot(l, n);

    float4 diffuse_color = float4(0.0, 0.0, 0.0, 0.0);
    if (HasDiffuseTexture) {
      diffuse_color = DiffuseTexture.Sample(ExperimentalSampler, input.TexCoord);
    } else {
      diffuse_color = DiffuseColor;
    }

    finalColor += PointLights[i].DiffuseColor * diffuse_color * max(nDotL, 0);
  }

  return finalColor;
}
