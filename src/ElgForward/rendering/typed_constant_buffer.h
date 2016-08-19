#pragma once

#include <string>

#include <d3d11.h>

#include "rendering/constant_buffer.h"

namespace Rendering {
namespace ConstantBuffer {

template<typename T>
class TypedHandle {
public:
  using StorageType = Handle::StorageType;
  using TagType = Handle::TagType;

  constexpr static const StorageType MaxIndex = Handle::MaxIndex;
  constexpr static const StorageType MaxGenerarion = Handle::MaxGenerarion;

  TypedHandle() : m_handle_() {
  }

  TypedHandle(StorageType index, StorageType generation) : m_handle_(index, generation) {
  }

  explicit TypedHandle(Handle handle) : m_handle_(handle) {
  }

  explicit operator Handle() const {
    return m_handle_;
  }

  StorageType GetIndex() {
    return m_handle_.GetIndex();
  }

  void SetIndex(StorageType index) {
    m_handle_.SetIndex(index);
  }

  StorageType GetGeneration() {
    return m_handle_.GetGeneration();
  }

  void GetGeneration(StorageType generation) {
    m_handle_.SetGeneration(generation);
  }

  void NextGeneration() {
    m_handle_.NextGeneration();
  }

  bool IsValid() {
    return m_handle_.IsValid();
  }

  StorageType CompactForm() {
    return m_handle_.CompactForm();
  }

private:
  Handle m_handle_;
};

template<typename T>
inline TypedHandle<T> Create(size_t cpu_name_hash, size_t gpu_name_hash, T* initial_data, ID3D11Device* device) {
  const auto& t_info = typeid(T);

  size_t type_hash = t_info.hash_code();
  size_t type_size = sizeof(T);
  size_t type_alignment = alignof(T);

  auto handle = Create(cpu_name_hash, gpu_name_hash, type_hash, type_size, type_alignment, initial_data, device);
  return TypedHandle<T>(handle);
}

template<typename T>
inline TypedHandle<T> Create(const std::string& cpu_name, const std::string& gpu_name, T* initial_data, ID3D11Device* device) {
  std::hash<std::string> hasher;
  auto handle = Create(hasher(cpu_name), hasher(gpu_name), initial_data, device);
  return TypedHandle<T>(handle);
}

template<typename T>
inline TypedHandle<T> Create(size_t name_hash, T* initial_data, ID3D11Device* device) {
  const auto& t_info = typeid(T);

  size_t type_hash = t_info.hash_code();
  size_t type_size = sizeof(T);
  size_t type_alignment = alignof(T);

  auto handle = Create(name_hash, type_hash, type_size, type_alignment, initial_data, device);
  return TypedHandle<T>(handle);
}

template<typename T>
inline TypedHandle<T> Create(const std::string& name, T* initial_data, ID3D11Device* device) {
  std::hash<std::string> hasher;
  auto handle = Create(hasher(name), initial_data, device);
  return TypedHandle<T>(handle);
}

template<typename T>
inline T* GetCpuBuffer(TypedHandle<T> handle) {
  return static_cast<T*>(GetCpuBuffer(static_cast<Handle>(handle)));
}

template<typename T>
inline Microsoft::WRL::ComPtr<ID3D11Buffer> GetGpuBuffer(TypedHandle<T> handle) {
  return GetGpuBuffer(static_cast<Handle>(handle));
}

template<typename T>
inline bool SendToGpu(TypedHandle<T> handle, ID3D11DeviceContext* device_context) {
  return SendToGpu(static_cast<Handle>(handle), device_context);
}

}  // namespace ConstantBuffer
}  // namespace Rendering