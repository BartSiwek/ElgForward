#ifndef ELGFORWARD_SHADERS_BASIC_H_
#define ELGFORWARD_SHADERS_BASIC_H_

struct VertexShaderInput {
  float3 PositionMs : POSITION;
  float3 NormalMs : NORMAL;
  float2 TexCoord : TEXCOORD0;
};

struct VertexShaderOutput {
  float4 PositionClipSpace : SV_Position;
  float4 PositionViewSpace : POSITION;
  float3 Normal : NORMAL;
  float2 TexCoord : TEXCOORD0;
  
};

#endif // ELGFORWARD_SHADERS_BASIC_H_
