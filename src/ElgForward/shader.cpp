#include "shader.h"

#include <d3d11.h>
#include <D3Dcompiler.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include <sstream>
#include <memory>

#include "filesystem.h"
#include "hash.h"
#include "resource_array.h"
#include "handle_cache.h"

ResourceArray<VertexShaderHandle, std::unique_ptr<VertexShader>, 255> g_vertex_shader_storage_;
HandleCache<size_t, VertexShaderHandle> g_vertex_shader_cache_;

ResourceArray<PixelShaderHandle, std::unique_ptr<PixelShader>, 255> g_pixel_shader_storage_;
HandleCache<size_t, PixelShaderHandle> g_pixel_shader_cache_;

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

bool ReflectShader(VertexShader* shader, const std::unordered_map<std::string, VertexDataChannel>& custom_channel_map) {
  Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflector;
  HRESULT reflector_creation_result = D3DReflect(shader->Buffer->GetBufferPointer(), shader->Buffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)reflector.GetAddressOf());
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

    shader->InputDescription.emplace_back(param_desc.SemanticName, param_desc.SemanticIndex, channel, component_count, param_desc.ComponentType);
  }

  return true;
}

VertexShaderHandle CreateVertexShader(const filesystem::path& path, const std::unordered_map<std::string, VertexDataChannel>& custom_channel_map, ID3D11Device* device) {
  std::hash<filesystem::path> hasher;
  size_t path_hash = hasher(path);

  auto cached_handle = g_vertex_shader_cache_.Get(path_hash);
  if (cached_handle.IsValid()) {
    return cached_handle;
  }

  auto vertex_shader = std::make_unique<VertexShader>();
  HRESULT load_result = D3DReadFileToBlob(path.c_str(), vertex_shader->Buffer.GetAddressOf());
  if (FAILED(load_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, load_result);
    return {};
  }

  HRESULT shader_creation_result = device->CreateVertexShader(vertex_shader->Buffer->GetBufferPointer(), 
                                                              vertex_shader->Buffer->GetBufferSize(), 
                                                              nullptr,
                                                              vertex_shader->Shader.GetAddressOf());
  
  if (FAILED(shader_creation_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, shader_creation_result);
    return {};
  }

  bool relfection_ok = ReflectShader(vertex_shader.get(), custom_channel_map);
  if (!relfection_ok) {
    return {};
  }

  auto new_handle = g_vertex_shader_storage_.Add(std::move(vertex_shader));
  g_vertex_shader_cache_.Set(path_hash, new_handle);
  return new_handle;
}

VertexShader* RetreiveVertexShader(VertexShaderHandle handle) {
  return g_vertex_shader_storage_.Get(handle).get();
}

PixelShaderHandle CreatePixelShader(const filesystem::path& path, ID3D11Device* device) {
  std::hash<filesystem::path> hasher;
  size_t path_hash = hasher(path);

  auto cached_handle = g_pixel_shader_cache_.Get(path_hash);
  if (cached_handle.IsValid()) {
    return cached_handle;
  }
  
  auto pixel_shader = std::make_unique<PixelShader>();
  HRESULT load_result = D3DReadFileToBlob(path.c_str(), pixel_shader->Buffer.GetAddressOf());
  if (FAILED(load_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, load_result);
    return {};
  }

  HRESULT shader_creation_result = device->CreatePixelShader(pixel_shader->Buffer->GetBufferPointer(),
                                                             pixel_shader->Buffer->GetBufferSize(),
                                                             nullptr,
                                                             pixel_shader->Shader.GetAddressOf());

  if (FAILED(shader_creation_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, shader_creation_result);
    return {};
  }

  auto new_handle = g_pixel_shader_storage_.Add(std::move(pixel_shader));
  g_pixel_shader_cache_.Set(path_hash, new_handle);
  return new_handle;
}

PixelShader* RetreivePixelShader(PixelShaderHandle handle) {
  return g_pixel_shader_storage_.Get(handle).get();
}
