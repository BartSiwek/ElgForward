#include "vertex_layout.h"

#include <vector>

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "hash.h"
#include "shader.h"
#include "resource_array.h"
#include "handle_cache.h"

ResourceArray<VertexLayoutHandle, Microsoft::WRL::ComPtr<ID3D11InputLayout>, 255> g_storage_;
HandleCache<std::vector<D3D11_INPUT_ELEMENT_DESC>, VertexLayoutHandle> g_cache_;

VertexLayoutHandle CreateVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout, ID3DBlob* shader_blob, ID3D11Device* device) {
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

Microsoft::WRL::ComPtr<ID3D11InputLayout> RetreiveVertexLayout(VertexLayoutHandle handle) {
  return g_storage_.Get(handle);
}
