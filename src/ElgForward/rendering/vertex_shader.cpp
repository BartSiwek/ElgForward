#include "rendering/vertex_shader.h"

#include <d3d11.h>
#include <D3Dcompiler.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include <sstream>
#include <memory>

#include "core/filesystem.h"
#include "core/hash.h"
#include "core/resource_array.h"
#include "core/handle_cache.h"

namespace Rendering {
namespace VertexShader {

Core::ResourceArray<Handle, ShaderData, 255> g_vertex_shader_storage_;
Core::HandleCache<size_t, Handle> g_vertex_shader_cache_;

const std::unordered_map<std::string, VertexDataChannel>& GetStandardChannelMap() {
  static std::unordered_map<std::string, VertexDataChannel> standard_channel_map = {
    { "POSITION0", VertexDataChannel::POSITIONS },
    { "NORMAL0", VertexDataChannel::NORMALS },
    { "TANGENT0", VertexDataChannel::TANGENTS },
    { "BITANGENT0", VertexDataChannel::BITANGENTS },
    { "TEXCOORD0", VertexDataChannel::TEXCOORDS0 },
    { "TEXCOORD1", VertexDataChannel::TEXCOORDS1 },
    { "TEXCOORD2", VertexDataChannel::TEXCOORDS2 },
    { "TEXCOORD3", VertexDataChannel::TEXCOORDS3 },
    { "TEXCOORD4", VertexDataChannel::TEXCOORDS4 },
    { "TEXCOORD5", VertexDataChannel::TEXCOORDS5 },
    { "TEXCOORD6", VertexDataChannel::TEXCOORDS6 },
    { "TEXCOORD7", VertexDataChannel::TEXCOORDS7 },
    { "COLOR0", VertexDataChannel::COLORS0 },
    { "COLOR1", VertexDataChannel::COLORS1 },
    { "COLOR2", VertexDataChannel::COLORS2 },
    { "COLOR3", VertexDataChannel::COLORS3 },
    { "COLOR4", VertexDataChannel::COLORS4 },
    { "COLOR5", VertexDataChannel::COLORS5 },
    { "COLOR6", VertexDataChannel::COLORS6 },
    { "COLOR7", VertexDataChannel::COLORS7 },
  };

  return standard_channel_map;
}

uint32_t GetComponentCount(BYTE mask) {
  uint32_t component_count = 0;
  while (mask != 0) {
    if (mask & 0x01) {
      component_count++;
    }
    mask = mask >> 1;
  }
  return component_count;
}

bool MapSemanticsToChannel(const char* semantic_name, uint32_t index, const std::unordered_map<std::string, VertexDataChannel>& custom_channel_map, VertexDataChannel* channel) {
  std::string full_name(semantic_name);
  full_name.append(std::to_string(index));

  auto custom_channel_it = custom_channel_map.find(full_name);
  if (custom_channel_it != std::end(custom_channel_map)) {
    *channel = custom_channel_it->second;
    return true;
  }

  auto standard_channel_map = GetStandardChannelMap();
  auto standard_channel_it = standard_channel_map.find(full_name);
  if (standard_channel_it != std::end(standard_channel_map)) {
    *channel = standard_channel_it->second;
    return true;
  }

  return false;
}

bool ReflectShader(ShaderData* data, const std::unordered_map<std::string, VertexDataChannel>& custom_channel_map) {
  Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflector;
  HRESULT reflector_creation_result = D3DReflect(data->Buffer->GetBufferPointer(), data->Buffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)reflector.GetAddressOf());
  if (FAILED(reflector_creation_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, reflector_creation_result);
    return false;
  }

  D3D11_SHADER_DESC shader_desc;
  HRESULT get_desc_result = reflector->GetDesc(&shader_desc);
  if (FAILED(get_desc_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, get_desc_result);
    return false;
  }

  D3D11_SIGNATURE_PARAMETER_DESC param_desc;
  for (uint32_t i = 0; i < shader_desc.InputParameters; ++i) {
    reflector->GetInputParameterDesc(i, &param_desc);

    auto component_count = GetComponentCount(param_desc.Mask);
    VertexDataChannel channel;
    bool map_ok = MapSemanticsToChannel(param_desc.SemanticName, param_desc.SemanticIndex, custom_channel_map, &channel);
    if (!map_ok) {
      return false;
    }

    data->InputDescription.emplace_back(param_desc.SemanticName, param_desc.SemanticIndex, channel, component_count, param_desc.ComponentType);
  }

  return true;
}

Handle Create(const filesystem::path& path, const std::unordered_map<std::string, VertexDataChannel>& custom_channel_map, ID3D11Device* device) {
  std::hash<filesystem::path> hasher;
  size_t path_hash = hasher(path);

  auto cached_handle = g_vertex_shader_cache_.Get(path_hash);
  if (cached_handle.IsValid()) {
    return cached_handle;
  }

  auto data = ShaderData();
  
  HRESULT load_result = D3DReadFileToBlob(path.c_str(), data.Buffer.GetAddressOf());
  if (FAILED(load_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, load_result);
    return {};
  }

  HRESULT shader_creation_result = device->CreateVertexShader(data.Buffer->GetBufferPointer(),
                                                              data.Buffer->GetBufferSize(),
                                                              nullptr,
                                                              data.Shader.GetAddressOf());
  
  if (FAILED(shader_creation_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, shader_creation_result);
    return {};
  }

  bool relfection_ok = ReflectShader(&data, custom_channel_map);
  if (!relfection_ok) {
    return {};
  }

  auto new_handle = g_vertex_shader_storage_.Add(std::move(data));
  g_vertex_shader_cache_.Set(path_hash, new_handle);
  return new_handle;
}

ShaderData* Retreive(Handle handle) {
  return &g_vertex_shader_storage_.Get(handle);
}

}  // namespace VertexShader
}  // namespace Rendering
