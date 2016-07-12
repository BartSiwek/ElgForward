#pragma once

#include <string>

#include <d3d11.h>

#include "handle.h"

namespace ConstantBuffer {

struct ConstantBufferTag {};

using ConstantBufferHandle = Handle<8, 24, ConstantBufferTag>;

ConstantBufferHandle Create(
    size_t name_hash,
    size_t type_hash,
    size_t type_size,
    size_t type_alignment,
    void* initial_data,
    ID3D11Device* device);

inline ConstantBufferHandle Create(
    const std::string& name,
    size_t type_hash,
    size_t type_size,
    size_t type_alignment,
    void* initial_data,
    ID3D11Device* device) {
  std::hash<std::string> hasher;
  return Create(hasher(name), type_hash, type_size, type_alignment, initial_data, device);
}

template<typename T>
inline ConstantBufferHandle Create(
    size_t name_hash,
    T* initial_data,
    ID3D11Device* device) {
  const auto& t_info = typeid(T);

  size_t type_hash = t_info.hash_code();
  size_t type_size = sizeof(T);
  size_t type_alignment = alignof(T);

  return Create(name_hash, type_hash, type_size, type_alignment, initial_data, device);
}

template<typename T>
inline ConstantBufferHandle Create(
    const std::string& name,
    T* initial_data,
    ID3D11Device* device) {
  std::hash<std::string> hasher;
  return Create(hasher(name), initial_data, device);
}

void* GetCpuBuffer(ConstantBufferHandle handle);

template<typename T>
inline T* GetCpuBuffer(ConstantBufferHandle handle) {
  return static_cast<T*>(GetCpuBuffer(handle));
}

ID3D11Buffer* GetGpuBuffer(ConstantBufferHandle handle);

ID3D11Buffer** GetAddressOfGpuBuffer(ConstantBufferHandle handle);

bool SendToGpu(ConstantBufferHandle handle, ID3D11DeviceContext* device_context);

}  // namespace ConstantBuffer
