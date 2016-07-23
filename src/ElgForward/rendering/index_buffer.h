#pragma once

#include <vector>

#include <d3d11.h>
#include <wrl.h>

#include "core/handle.h"

namespace Rendering {
namespace IndexBuffer {

struct IndexBufferTag {};

using Handle = Core::Handle<8, 24, IndexBufferTag>;

Handle Create(size_t hash, const void* data, size_t data_size, ID3D11Device* device);

Microsoft::WRL::ComPtr<ID3D11Buffer> Retreive(Handle handle);

inline Handle Create(const std::string& key, const void* data, size_t data_size, ID3D11Device* device) {
  std::hash<std::string> hasher;
  return Create(hasher(key), data, data_size, device);
}

template<typename IndexType>
inline Handle Create(size_t hash, const std::vector<IndexType>& data, ID3D11Device* device) {
  return Create(hash, &data[0], data.size() * sizeof(IndexType), device);
}

template<typename IndexType>
inline Handle Create(const std::string& key, const std::vector<IndexType>& data, ID3D11Device* device) {
  return Create(key, &data[0], data.size() * sizeof(IndexType), device);
}

}  // namespace IndexBuffer
}  // namespace Rendering
