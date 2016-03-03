#include "hlsl_definitions.h"

cbuffer PerFrameConstants  : register(PER_FRAME_CB_REGISTER) {
  float4x4 ModelViewMatrix;
};

struct VertexShaderInput {
  float3 Position : POSITION;
  float3 Normal : NORMAL;
  float2 TexCoord : TEXCOORD0;
};

struct VertexShaderOutput {
  float4 Position : POSITION;
  float4 Color : COLOR;
};

VertexShaderOutput main(VertexShaderInput input) {
  VertexShaderOutput output;

  float4 position = float4(input.Position, 1.0);

  output.Position = float4((mul(position, ModelViewMatrix).xyz * 0.5) + float3(0, 0, 0.5), 1);
  output.Color = float4(0.5, 0.5, 0.5, 1);

  return output;
}
