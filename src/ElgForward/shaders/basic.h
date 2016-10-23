#ifndef ELGFORWARD_SHADERS_BASIC_H_
#define ELGFORWARD_SHADERS_BASIC_H_

#include "registers.h"

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

cbuffer PerFrameConstants : PER_FRAME_CONSTANT_BUFFER_REGISTER{
  int DirectionalLightCount;
int SpotLightCount;
int PointLightCount;
float pad;
};

cbuffer PerCamersConstants : PER_CAMERA_CONSTANT_BUFFER_REGISTER{
  float4x4 ViewMatrix;
float4x4 ViewMatrixInverseTranspose;
float4x4 ProjectionMatrix;
}

cbuffer PerObjectConstants : PER_OBJECT_CONSTANT_BUFFER_REGISTER{
  float4x4 ModelMatrix;
float4x4 ModelMatrixInverseTranspose;
}

#endif // ELGFORWARD_SHADERS_BASIC_H_
