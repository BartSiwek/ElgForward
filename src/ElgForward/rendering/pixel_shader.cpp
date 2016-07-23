#include "rendering/pixel_shader.h"

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
namespace PixelShader {

Core::ResourceArray<Handle, ShaderData, 255> g_pixel_shader_storage_;
Core::HandleCache<size_t, Handle> g_pixel_shader_cache_;

Handle Create(const filesystem::path& path, ID3D11Device* device) {
  std::hash<filesystem::path> hasher;
  size_t path_hash = hasher(path);

  auto cached_handle = g_pixel_shader_cache_.Get(path_hash);
  if (cached_handle.IsValid()) {
    return cached_handle;
  }
  
  auto data = ShaderData();
  HRESULT load_result = D3DReadFileToBlob(path.c_str(), data.Buffer.GetAddressOf());
  if (FAILED(load_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, load_result);
    return {};
  }

  HRESULT shader_creation_result = device->CreatePixelShader(data.Buffer->GetBufferPointer(),
                                                             data.Buffer->GetBufferSize(),
                                                             nullptr,
                                                             data.Shader.GetAddressOf());

  if (FAILED(shader_creation_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, shader_creation_result);
    return {};
  }

  auto new_handle = g_pixel_shader_storage_.Add(std::move(data));
  g_pixel_shader_cache_.Set(path_hash, new_handle);
  return new_handle;
}

ShaderData* Retreive(Handle handle) {
  return &g_pixel_shader_storage_.Get(handle);
}

}  // namespace PixelShader
}  // namespace Rendering
