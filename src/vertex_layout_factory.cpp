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

class CachedResourceArray {
 public:
  using HandleType = VertexLayoutHandle;
  using Key = std::vector<D3D11_INPUT_ELEMENT_DESC>;
  using Value = Microsoft::WRL::ComPtr<ID3D11InputLayout>;

  constexpr static const size_t Size = 255;

  template<typename F>
  HandleType Add(const Key& key, const F& factory) {
    return m_cache_.GetOrAdd(key, [&]() {
      auto product = factory();

      if (product.second) {
        auto result = m_storage_.Add(product.first);
        return result;
      }
      return HandleType();
    });
  }

  Value Get(VertexLayoutHandle handle) {
    if (m_storage_.IsActive(handle)) {
      return m_storage_.Get(handle);
    }
    return Value();
  }

 private:
   ResourceArray<HandleType, Value, Size> m_storage_;
   HandleCache<Key, HandleType> m_cache_;
};

CachedResourceArray g_storage_;

VertexLayoutHandle CreateVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout, ID3DBlob* shader_blob, ID3D11Device* device) {
  auto factory = [&]() {
    Microsoft::WRL::ComPtr<ID3D11InputLayout> vertex_layout;
    auto create_input_layout_result = device->CreateInputLayout(&input_layout[0],
                                                                input_layout.size(),
                                                                shader_blob->GetBufferPointer(),
                                                                shader_blob->GetBufferSize(),
                                                                vertex_layout.GetAddressOf());

    if (FAILED(create_input_layout_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, create_input_layout_result);
      return std::make_pair(Microsoft::WRL::ComPtr<ID3D11InputLayout>(), false);
    }
    return std::make_pair(vertex_layout, true);
  };

  return g_storage_.Add(input_layout, factory);
};

ID3D11InputLayout* GetVertexLayoutFromFactory(VertexLayoutHandle handle) {
  return g_storage_.Get(handle).Get();
}
