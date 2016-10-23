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
#include "rendering/shader_reflection.h"

namespace Rendering {
namespace VertexShader {

Core::ResourceArray<Handle, ShaderData, 255> g_vertex_shader_storage_;
Core::HandleCache<size_t, Handle> g_vertex_shader_cache_;

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

  bool input_reflection_ok = ShaderReflection::ReflectInputs(data.Buffer.Get(), custom_channel_map, &data.ReflectionData);
  if (!input_reflection_ok) {
    return {};
  }

  bool texture_reflection_ok = ShaderReflection::ReflectTextures(data.Buffer.Get(), &data.ReflectionData);
  if (!texture_reflection_ok) {
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
