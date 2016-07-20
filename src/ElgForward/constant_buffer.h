#pragma once

#include <string>

#include <d3d11.h>

#include "handle.h"

namespace ConstantBuffer {

struct ConstantBufferTag {};

using Handle = Core::Handle<8, 24, ConstantBufferTag>;

/* Basic interface */
Handle Create(
    size_t name_hash,
    size_t type_hash,
    size_t type_size,
    size_t type_alignment,
    void* initial_data,
    ID3D11Device* device);

void* GetCpuBuffer(Handle handle);

ID3D11Buffer* GetGpuBuffer(Handle handle);

ID3D11Buffer** GetAddressOfGpuBuffer(Handle handle);

bool SendToGpu(Handle handle, ID3D11DeviceContext* device_context);

/* Extended interface */
inline Handle Create(
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
inline Handle Create(
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
inline Handle Create(
    const std::string& name,
    T* initial_data,
    ID3D11Device* device) {
  std::hash<std::string> hasher;
  return Create(hasher(name), initial_data, device);
}

template<typename T>
inline T* GetCpuBuffer(Handle handle) {
  return static_cast<T*>(GetCpuBuffer(handle));
}

}  // namespace ConstantBuffer
