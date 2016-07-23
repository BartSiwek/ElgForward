#include "vertex_layout.h"

#include <vector>

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "core/hash.h"
#include "shader.h"
#include "core/resource_array.h"
#include "core/handle_cache.h"

namespace Rendering {
namespace VertexLayout {

Core::ResourceArray<Handle, Microsoft::WRL::ComPtr<ID3D11InputLayout>, 255> g_storage_;
Core::HandleCache<std::vector<D3D11_INPUT_ELEMENT_DESC>, Handle> g_cache_;

Handle Create(const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout, ID3DBlob* shader_blob, ID3D11Device* device) {
  auto cached_handle = g_cache_.Get(input_layout);
  if (cached_handle.IsValid()) {
    return cached_handle;
  }

  Microsoft::WRL::ComPtr<ID3D11InputLayout> vertex_layout;
  auto create_input_layout_result = device->CreateInputLayout(&input_layout[0],
                                                              input_layout.size(),
                                                              shader_blob->GetBufferPointer(),
                                                              shader_blob->GetBufferSize(),
                                                              vertex_layout.GetAddressOf());

  if (FAILED(create_input_layout_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, create_input_layout_result);
    return {};
  }

  auto new_handle = g_storage_.Add(vertex_layout);
  g_cache_.Set(input_layout, new_handle);
  return new_handle;
};

Microsoft::WRL::ComPtr<ID3D11InputLayout> Retreive(Handle handle) {
  return g_storage_.Get(handle);
}

}  // namespace VertexLayout
}  // namespace Rendering
