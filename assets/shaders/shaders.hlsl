#include "hlsl_definitions.h"

cbuffer PerFrameConstants  : register(PER_FRAME_CB_REGISTER) {
  float4x4 ModelViewMatrix;
};

struct VertexShaderInput {
  float4 Position : SV_Position;
  float4 Normal : NORMAL;
  float4 TexCoord : TEXCOORD0;
};

struct VertexShaderOutput {
  float4 Position : SV_POSITION;
  float4 Color : COLOR;
};

VertexShaderOutput VS(VertexShaderInput input) {
  VertexShaderOutput output;

  output.Position = float4((mul(input.Position, ModelViewMatrix).xyz * 0.5) + float3(0, 0, 0.5), 1);
  output.Color = float4(0.5, 0.5, 0.5, 1);

  return output;
}

float4 PS(VertexShaderOutput input) : SV_TARGET {
  return input.Color;
}