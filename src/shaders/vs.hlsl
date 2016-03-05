#include "hlsl_definitions.h"

cbuffer PerFrameConstants  : register(PER_FRAME_CB_REGISTER) {
  float4x4 ModelMatrix;
  float4x4 ViewMatrix;
  float4x4 ModelViewMatrix;
  float4x4 NormalMatrix;
};

static float4 LightPositionWs = float4(0, 0, 0, -0.5);

static float4 LightDiffuseColor = float4(0.8, 0.8, 0.8, 1);
static float4 MaterialDiffuseColor = float4(0.2, 0.4, 0.8, 1);

struct VertexShaderInput {
  float3 PositionMs : POSITION;
  float3 NormalMs : NORMAL;
  float2 TexCoord : TEXCOORD0;
};

struct VertexShaderOutput {
  float4 PositionVs : SV_Position;
  float4 Color : COLOR;
};

VertexShaderOutput main(VertexShaderInput input) {
  VertexShaderOutput output;

  float4 positionMs = float4(input.PositionMs, 1.0);
  output.PositionVs = mul(ModelViewMatrix, positionMs);

  float4 lightPositionVs = mul(ViewMatrix, LightPositionWs);

  float3 lu = lightPositionVs - output.PositionVs;
  float3 l = normalize(lu);

  float3 n = normalize(mul((float3x3)NormalMatrix, input.NormalMs));

  output.Color = LightDiffuseColor * MaterialDiffuseColor * max(dot(n, l), 0);

  return output;
}
