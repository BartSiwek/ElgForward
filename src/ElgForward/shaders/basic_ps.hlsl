#pragma pack_matrix(row_major)

#include "basic.h"

float4 main(VertexShaderOutput input) : SV_TARGET {
  return input.Color;
}
