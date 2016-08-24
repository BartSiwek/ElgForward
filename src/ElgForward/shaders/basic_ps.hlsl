#pragma pack_matrix(row_major)

#include "registers.h"
#include "basic.h"

cbuffer PerFrameConstants : PER_FRAME_CONSTANT_BUFFER_REGISTER {
  int DirectionalLightCount;
  int SpotLightCount;
  int PointLightCount;
  float pad;
};

cbuffer PerCamersConstants : PER_CAMERA_CONSTANT_BUFFER_REGISTER {
  float4x4 ViewMatrix;
  float4x4 ViewMatrixInverseTranspose;
  float4x4 ProjectionMatrix;
}

cbuffer PerObjectConstants : PER_OBJECT_CONSTANT_BUFFER_REGISTER {
  float4x4 ModelMatrix;
  float4x4 ModelMatrixInverseTranspose;
}

cbuffer PerMaterialConstants : PER_MATERIAL_CONSTANT_BUFFER_REGISTER {
  float4 DiffuseColor;
  float4 SpecularColor;
  float SpecularPower;
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

Texture2D<float4> ExperimentalTexture : EXPERIMENTAL_TEXTURE_REGISTER;
SamplerState ExperimentalSampler;

float4 main(VertexShaderOutput input) : SV_TARGET {
  float3 n = normalize(input.Normal);

  float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
  for (int i = 0; i < PointLightCount; ++i) {
    float3 lu = (PointLights[i].PositionViewSpace - input.PositionViewSpace).xyz;
    float3 l = normalize(lu);

    float nDotL = dot(l, n);

    finalColor += PointLights[i].DiffuseColor * DiffuseColor * ExperimentalTexture.Sample(ExperimentalSampler, input.TexCoord) * max(nDotL, 0);
  }

  return finalColor;
}
