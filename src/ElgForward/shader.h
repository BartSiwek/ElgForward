#pragma once

#include <vector>
#include <unordered_map>

#include <d3d11.h>
#include <wrl.h>

#include "filesystem.h"
#include "vertex_data.h"
#include "handle.h"

struct VertexShaderInputDescription {
  VertexShaderInputDescription(const char* name, uint32_t index, VertexDataChannel channel, uint32_t component_count, D3D_REGISTER_COMPONENT_TYPE component_type)
      : SemanticName(name),
        SemanticIndex(index),
        Channel(channel),
        ComponentCount(component_count),
        ComponentType(component_type) {
  }

  std::string SemanticName = "";
  uint32_t SemanticIndex = 0;
  VertexDataChannel Channel = VertexDataChannel::UNKNOWN;
  uint32_t ComponentCount = 0;
  D3D_REGISTER_COMPONENT_TYPE ComponentType = D3D_REGISTER_COMPONENT_UNKNOWN;
};

struct VertexShader {
  VertexShader() = default;
  ~VertexShader() = default;

  VertexShader(const VertexShader&) = delete;
  VertexShader& operator=(const VertexShader&) = delete;

  VertexShader(VertexShader&&) = default;
  VertexShader& operator=(VertexShader&&) = default;

  Microsoft::WRL::ComPtr<ID3DBlob> Buffer = nullptr;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> Shader = nullptr;
  std::vector<VertexShaderInputDescription> InputDescription = {};
};

struct PixelShader {
  PixelShader() = default;
  ~PixelShader() = default;

  PixelShader(const PixelShader&) = delete;
  PixelShader& operator=(const PixelShader&) = delete;

  PixelShader(PixelShader&&) = default;
  PixelShader& operator=(PixelShader&&) = default;

  Microsoft::WRL::ComPtr<ID3DBlob> Buffer = nullptr;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> Shader = nullptr;
};

struct VertexShaderTag {};

using VertexShaderHandle = Handle<8, 24, VertexShaderTag>;

struct PixelShaderTag {};

using PixelShaderHandle = Handle<8, 24, PixelShaderTag>;

VertexShaderHandle CreateVertexShader(const filesystem::path& path, const std::unordered_map<std::string, VertexDataChannel>& custom_channel_map, ID3D11Device* device);

VertexShader* RetreiveVertexShader(VertexShaderHandle handle);

PixelShaderHandle CreatePixelShader(const filesystem::path& path, ID3D11Device* device);

PixelShader* RetreivePixelShader(PixelShaderHandle handle);
