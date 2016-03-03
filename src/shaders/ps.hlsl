#include "hlsl_definitions.h"

struct VertexShaderOutput {
  float4 Position : POSITION;
  float4 Color : COLOR;
};

float4 main(VertexShaderOutput input) : SV_TARGET {
  return input.Color;
}