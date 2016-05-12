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

namespace std {

template<>
struct hash<DXGI_FORMAT> {
  size_t operator()(const DXGI_FORMAT& f) const {
    return static_cast<size_t>(f);
  }
};

template<>
struct hash<D3D11_INPUT_CLASSIFICATION> {
  size_t operator()(const D3D11_INPUT_CLASSIFICATION& c) const {
    return static_cast<size_t>(c);
  }
};

template<>
struct hash<D3D11_INPUT_ELEMENT_DESC> {
  size_t operator()(const D3D11_INPUT_ELEMENT_DESC& d) const {
    size_t seed = 0;

    hash_combine(seed, d.SemanticName);
    hash_combine(seed, d.SemanticIndex);
    hash_combine(seed, d.Format);
    hash_combine(seed, d.InputSlot);
    hash_combine(seed, d.AlignedByteOffset);
    hash_combine(seed, d.InputSlotClass);
    hash_combine(seed, d.InstanceDataStepRate);

    return seed;
  }
};

template<>
struct hash<std::vector<D3D11_INPUT_ELEMENT_DESC>> {
  size_t operator()(const std::vector<D3D11_INPUT_ELEMENT_DESC>& v) const {
    return hash_range(std::begin(v), std::end(v));
  }
};

template<>
struct equal_to<D3D11_INPUT_ELEMENT_DESC> {
  bool operator()(const D3D11_INPUT_ELEMENT_DESC& lhs, const D3D11_INPUT_ELEMENT_DESC& rhs) const {
    auto semantic_name_comparison_result = lstrcmpA(lhs.SemanticName, rhs.SemanticName);
    if (semantic_name_comparison_result == 0) {
      return lhs.SemanticIndex == rhs.SemanticIndex
          && lhs.Format == rhs.Format
          && lhs.InputSlot == rhs.InputSlot
          && lhs.AlignedByteOffset == rhs.AlignedByteOffset
          && lhs.InputSlotClass == rhs.InputSlotClass
          && lhs.InstanceDataStepRate == rhs.InstanceDataStepRate;
    }            
    return false;
  }
};

template<>
struct equal_to<std::vector<D3D11_INPUT_ELEMENT_DESC>> {
  bool operator()(const std::vector<D3D11_INPUT_ELEMENT_DESC>& lhs, const std::vector<D3D11_INPUT_ELEMENT_DESC>& rhs) const {
    return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), std::equal_to<D3D11_INPUT_ELEMENT_DESC>());
  }
};

}  // std

using Key = std::vector<D3D11_INPUT_ELEMENT_DESC>;
using Value = ResourceArray::HandleType;
using Cache = std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>>;

ResourceArray g_storage_;
Cache g_cache_;

ResourceArray::HandleType CreateVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout, ID3DBlob* shader_blob, ID3D11Device* device) {
  auto it = g_cache_.find(input_layout);

  if (it != std::end(g_cache_)) {
    return it->second;
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

  auto result = g_storage_.Add(vertex_layout);
  g_cache_[input_layout] = result;
  return result;
};

ID3D11InputLayout* GetVertexLayoutFromFactory(ResourceArray::HandleType handle) {
  if (g_storage_.IsActive(handle)) {
    return g_storage_.Get(handle).Get();
  }
  return nullptr;
}
