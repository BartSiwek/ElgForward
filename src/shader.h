#pragma once

#include <vector>

#include <d3d11.h>
#include <wrl.h>

#include "filesystem.h"

struct VertexShaderInputDescription {
  VertexShaderInputDescription(const char* name, uint32_t index, DXGI_FORMAT format) : SemanticName(name), SemanticIndex(index), Format(format) {
  }

  std::string SemanticName;
  uint32_t SemanticIndex;
  DXGI_FORMAT Format;
};

struct VertexShader {
  Microsoft::WRL::ComPtr<ID3DBlob> Buffer;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> Shader;
  std::vector<VertexShaderInputDescription> InputDescription;
};

struct PixelShader {
  Microsoft::WRL::ComPtr<ID3DBlob> Buffer;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> Shader;
};

bool LoadVertexShader(const filesystem::path& path, ID3D11Device* device, VertexShader* vertex_shader);

bool LoadPixelShader(const filesystem::path& path, ID3D11Device* device, PixelShader* pixel_shader);
