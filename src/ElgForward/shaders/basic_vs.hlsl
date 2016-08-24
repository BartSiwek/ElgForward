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

VertexShaderOutput main(VertexShaderInput input) {
  VertexShaderOutput output;

  float4x4 modelViewMatrix = mul(ModelMatrix, ViewMatrix);
  float4x4 modelViewProjectionMatrix = mul(modelViewMatrix, ProjectionMatrix);
  float4x4 modelViewMatrixInverseTranspose = mul(ModelMatrixInverseTranspose, ViewMatrixInverseTranspose);

  float4 positionMs = float4(input.PositionMs, 1.0);
  output.PositionClipSpace = mul(positionMs, modelViewProjectionMatrix);
  output.PositionViewSpace = mul(positionMs, modelViewMatrix);
  output.Normal = mul(input.NormalMs, (float3x3)modelViewMatrixInverseTranspose);
  output.TexCoord = input.TexCoord;

  return output;
}
