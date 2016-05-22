#pragma once

#include <vector>

#include <d3d11.h>
#include <wrl.h>

#include "handle.h"

struct VertexBufferTag {};

using VertexBufferHandle = Handle<8, 24, VertexBufferTag>;

VertexBufferHandle CreateVertexBuffer(size_t hash, const void* data, size_t data_size, ID3D11Device* device);

Microsoft::WRL::ComPtr<ID3D11Buffer> RetreiveVertexBuffer(VertexBufferHandle handle);

inline VertexBufferHandle CreateVertexBuffer(const std::string& key, const void* data, size_t data_size, ID3D11Device* device) {
  std::hash<std::string> hasher;
  return CreateVertexBuffer(hasher(key), data, data_size, device);
}

template<typename EntryType>
inline VertexBufferHandle CreateVertexBuffer(size_t hash, const std::vector<EntryType>& data, ID3D11Device* device) {
  return CreateVertexBuffer(hash, &data[0], data.size() * sizeof(EntryType), device);
}

template<typename EntryType>
inline VertexBufferHandle CreateVertexBuffer(const std::string& key, const std::vector<EntryType>& data, ID3D11Device* device) {
  return CreateVertexBuffer(key, &data[0], data.size() * sizeof(EntryType), device);
}


