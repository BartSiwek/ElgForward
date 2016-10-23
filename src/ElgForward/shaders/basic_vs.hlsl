#pragma pack_matrix(row_major)

#include "registers.h"
#include "basic.h"

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
