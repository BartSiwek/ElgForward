#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "filesystem.h"

struct VertexShader {
  Microsoft::WRL::ComPtr<ID3DBlob> Buffer;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> Shader;
};

struct PixelShader {
  Microsoft::WRL::ComPtr<ID3DBlob> Buffer;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> Shader;
};

bool LoadVertexShader(const filesystem::path& path, ID3D11Device* device, VertexShader* vertex_shader,
                      const char* entry_point, const char* target);

bool LoadPixelShader(const filesystem::path& path, ID3D11Device* device, PixelShader* pixel_shader,
                     const char* entry_point, const char* target);
