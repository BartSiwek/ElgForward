#pragma once

#include <string>

#include <d3d11.h>

#include "handle.h"

namespace Rendering {
namespace StructuredBuffer {

struct StructuredBufferTag {};

using Handle = Core::Handle<8, 24, StructuredBufferTag>;

/* Basic interface */
Handle Create(
  size_t name_hash,
  size_t type_hash,
  size_t type_size,
  size_t type_alignment,
  size_t max_size,
  void* initial_data,
  size_t initial_count,
  ID3D11Device* device);

void* GetCpuBuffer(Handle handle);

void* GetElementAt(Handle handle, size_t index);

ID3D11Buffer* GetGpuBuffer(Handle handle);

ID3D11Buffer** GetAddressOfGpuBuffer(Handle handle);

ID3D11ShaderResourceView* GetShaderResourceView(Handle handle);

ID3D11ShaderResourceView** GetAddressOfShaderResourceView(Handle handle);

void SetCurrentSize(Handle handle, size_t new_size);

size_t GetCurrentSize(Handle handle);

bool Add(Handle handle, void* value);

size_t GetMaxSize(Handle handle);

bool SendToGpu(Handle handle, ID3D11DeviceContext* device_context);

/* Extended interface */
inline Handle Create(
  const std::string& name,
  size_t type_hash,
  size_t type_size,
  size_t type_alignment,
  size_t max_size,
  void* initial_data,
  size_t initial_count,
  ID3D11Device* device) {
  std::hash<std::string> hasher;
  return Create(hasher(name), type_hash, type_size, type_alignment, max_size, initial_data, initial_count, device);
}

template<typename T>
inline Handle Create(
  size_t name_hash,
  size_t max_size,
  T* initial_data,
  size_t initial_count,
  ID3D11Device* device) {
  const auto& t_info = typeid(T);

  size_t type_hash = t_info.hash_code();
  size_t type_size = sizeof(T);
  size_t type_alignment = alignof(T);

  return Create(name_hash, type_hash, type_size, type_alignment, max_size, initial_data, initial_count, device);
}

template<typename T>
inline Handle Create(
  const std::string& name,
  size_t max_size,
  T* initial_data,
  size_t initial_count,
  ID3D11Device* device) {
  std::hash<std::string> hasher;
  return Create(hasher(name), max_size, initial_data, initial_count, device);
}

template<typename T>
inline T* GetCpuBuffer(Handle handle) {
  return static_cast<T*>(GetCpuBuffer(handle));
}

template<typename T>
inline T* GetElementAt(Handle handle, size_t index) {
  return static_cast<T*>(GetElementAt(handle, index));
}

}  // namespace StructuredBuffer
}  // namespace Rendering