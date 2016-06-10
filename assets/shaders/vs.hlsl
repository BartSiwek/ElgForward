#pragma pack_matrix(row_major)

cbuffer PerFrameConstants  : register(b0) {
  float4x4 ModelMatrix;
  float4x4 ViewMatrix;
  float4x4 ProjectionMatrix;
  float4x4 NormalMatrix;
  float4x4 ModelViewMatrix;
  float4x4 ModelViewProjectionMatrix;
};

static float4 LightPositionWs = float4(0, 0, -5, 1);

static float4 LightDiffuseColor = float4(0.8, 0.8, 0.8, 1);
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

  float4 lightPositionVs = mul(LightPositionWs, ViewMatrix);

  float3 lu = (lightPositionVs - positionVs).xyz;
  float3 l = normalize(lu);

  float3 nu = mul(input.NormalMs, (float3x3)NormalMatrix);
  float3 n = normalize(nu);

  float nDotL = dot(l, n);

  float4 finalColor = LightDiffuseColor * MaterialDiffuseColor * max(nDotL, 0);

  output.Position = mul(positionMs, ModelViewProjectionMatrix);
  // output.Position = mul(positionMs, ModelViewMatrix);
  output.Color = finalColor;

  return output;
}
