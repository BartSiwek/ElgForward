#include "vertex_layout_factory.h"

#include <array>
#include <vector>
#include <unordered_set>

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "hash.h"
#include "shader.h"
#include "resource_array.h"
#include "handle_cache.h"

ResourceArray<VertexLayoutHandle, Microsoft::WRL::ComPtr<ID3D11InputLayout>, 255> g_storage_;
HandleCache<std::vector<D3D11_INPUT_ELEMENT_DESC>, VertexLayoutHandle> g_cache_;

/*
class CachedResourceArray {
public:

 private:
  ResourceArray m_storage_;
  HandleCache<Key, Value> m_cache_;
};
*/

VertexLayoutHandle CreateVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout, ID3DBlob* shader_blob, ID3D11Device* device) {
  return g_cache_.Get(input_layout, [&](){
    Microsoft::WRL::ComPtr<ID3D11InputLayout> vertex_layout;
    auto create_input_layout_result = device->CreateInputLayout(&input_layout[0],
                                                                input_layout.size(),
                                                                shader_blob->GetBufferPointer(),
                                                                shader_blob->GetBufferSize(),
                                                                vertex_layout.GetAddressOf());

    if (FAILED(create_input_layout_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, create_input_layout_result);
      return VertexLayoutHandle{};
    }

    auto result = g_storage_.Add(vertex_layout);
    return result;
  });
};

ID3D11InputLayout* GetVertexLayoutFromFactory(VertexLayoutHandle handle) {
  if (g_storage_.IsActive(handle)) {
    return g_storage_.Get(handle).Get();
  }
  return nullptr;
}
