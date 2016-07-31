#pragma pack_matrix(row_major)

#include "basic.h"

cbuffer PerFrameConstants : register(b0) {
  float4x4 ModelMatrix;
  float4x4 ModelMatrixInverseTranspose;
  float4x4 ViewMatrix;
  float4x4 ViewMatrixInverseTranspose;
  float4x4 ProjectionMatrix;
  float4x4 ModelViewMatrix;
  float4x4 ModelViewMatrixInverseTranspose;
  float4x4 ModelViewProjectionMatrix;
};

cbuffer LightData : register(b1) {
  int DirectionalLightCount;
  int SpotLightCount;
  int PointLightCount;
  float pad;
};

cbuffer Material : register(b2) {
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

StructuredBuffer <PointLight> PointLights : register(t0);

struct VertexShaderInput {
  float3 PositionMs : POSITION;
  float3 NormalMs : NORMAL;
  float2 TexCoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input) {
  VertexShaderOutput output;

  float4 positionMs = float4(input.PositionMs, 1.0);
  float4 positionVs = mul(positionMs, ModelViewMatrix);

  float3 nu = mul(input.NormalMs, (float3x3)ModelViewMatrixInverseTranspose);
  float3 n = normalize(nu);

  float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
  for (int i = 0; i < PointLightCount; ++i) {
    float4 lightPositionVs = mul(PointLights[i].PositionWorldSpace, ViewMatrix);

    float3 lu = (lightPositionVs - positionVs).xyz;
    float3 l = normalize(lu);

    float nDotL = dot(l, n);

    finalColor += PointLights[i].DiffuseColor * DiffuseColor * max(nDotL, 0);
  }

  output.Position = mul(positionMs, ModelViewProjectionMatrix);
  output.Color = finalColor;

  return output;
}
