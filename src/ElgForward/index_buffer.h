#pragma once

#include <vector>

#include <d3d11.h>
#include <wrl.h>

#include "handle.h"

struct IndexBufferTag {};

using IndexBufferHandle = Core::Handle<8, 24, IndexBufferTag>;

IndexBufferHandle CreateIndexBuffer(size_t hash, const void* data, size_t data_size, ID3D11Device* device);

Microsoft::WRL::ComPtr<ID3D11Buffer> RetreiveIndexBuffer(IndexBufferHandle handle);

inline IndexBufferHandle CreateIndexBuffer(const std::string& key, const void* data, size_t data_size, ID3D11Device* device) {
  std::hash<std::string> hasher;
  return CreateIndexBuffer(hasher(key), data, data_size, device);
}

template<typename IndexType>
inline IndexBufferHandle CreateIndexBuffer(size_t hash, const std::vector<IndexType>& data, ID3D11Device* device) {
  return CreateIndexBuffer(hash, &data[0], data.size() * sizeof(IndexType), device);
}

template<typename IndexType>
inline IndexBufferHandle CreateIndexBuffer(const std::string& key, const std::vector<IndexType>& data, ID3D11Device* device) {
  return CreateIndexBuffer(key, &data[0], data.size() * sizeof(IndexType), device);
}
