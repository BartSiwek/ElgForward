#pragma once

#include <vector>
#include <unordered_map>

#include <d3d11.h>
#include <wrl.h>

#include "core/filesystem.h"
#include "core/handle.h"
#include "vertex_data.h"

namespace Rendering {
namespace VertexShader {

struct VertexShaderTag {};

using Handle = Core::Handle<8, 24, VertexShaderTag>;

struct InputDescription {
  InputDescription(const char* name, uint32_t index, VertexDataChannel channel, uint32_t component_count, D3D_REGISTER_COMPONENT_TYPE component_type)
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

struct ShaderData {
  ShaderData() = default;
  ~ShaderData() = default;

  ShaderData(const ShaderData&) = delete;
  ShaderData& operator=(const ShaderData&) = delete;

  ShaderData(ShaderData&&) = default;
  ShaderData& operator=(ShaderData&&) = default;

  Microsoft::WRL::ComPtr<ID3DBlob> Buffer = nullptr;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> Shader = nullptr;
  std::vector<InputDescription> InputDescription = {};
};

Handle Create(const filesystem::path& path, const std::unordered_map<std::string, VertexDataChannel>& custom_channel_map, ID3D11Device* device);

ShaderData* Retreive(Handle handle);

}  // namespace VertexShader
}  // namespace Rendering
