#pragma pack_matrix(row_major)

cbuffer PerFrameConstants  : register(b0) {
  float4x4 ModelMatrix;
  float4x4 ModelMatrixInverseTranspose;
  float4x4 ViewMatrix;
  float4x4 ViewMatrixInverseTranspose;
  float4x4 ProjectionMatrix;
  float4x4 ModelViewMatrix;
  float4x4 ModelViewMatrixInverseTranspose;
  float4x4 ModelViewProjectionMatrix;
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

static float4 MaterialDiffuseColor = float4(0.2, 0.4, 0.8, 1);

struct VertexShaderInput {
  float3 PositionMs : POSITION;
  float3 NormalMs : NORMAL;
  float2 TexCoord : TEXCOORD0;
};

struct VertexShaderOutput {
  float4 Position : SV_Position;
  float4 Color : COLOR;
};

VertexShaderOutput main(VertexShaderInput input) {
  VertexShaderOutput output;

  float4 positionMs = float4(input.PositionMs, 1.0);
  float4 positionVs = mul(positionMs, ModelViewMatrix);

  float4 lightPositionVs = mul(PointLights[0].PositionWorldSpace, ViewMatrix);

  float3 lu = (lightPositionVs - positionVs).xyz;
  float3 l = normalize(lu);

  float3 nu = mul(input.NormalMs, (float3x3)ModelViewMatrixInverseTranspose);
  float3 n = normalize(nu);

  float nDotL = dot(l, n);

  float4 finalColor = PointLights[0].DiffuseColor * MaterialDiffuseColor * max(nDotL, 0);

  output.Position = mul(positionMs, ModelViewProjectionMatrix);
  output.Color = finalColor;

  return output;
}
