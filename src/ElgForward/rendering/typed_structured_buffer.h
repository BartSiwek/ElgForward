#pragma once

#include <string>

#include <d3d11.h>

#include "rendering/structured_buffer.h"
#include "rendering/lights/point_light.h"

namespace Rendering {
namespace StructuredBuffer {

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

  T* begin() {
    return static_cast<T*>(GetElementAt(static_cast<Handle>(m_handle_), 0));
  }

  T* end() {
    size_t current_size = GetCurrentSize(static_cast<Handle>(m_handle_));
    return static_cast<T*>(GetElementAt(static_cast<Handle>(m_handle_), 0)) + current_size;
  }

  const T* cbegin() const {
    return static_cast<T*>(GetElementAt(static_cast<Handle>(m_handle_), 0));
  }

  const T* cend() const {
    size_t current_size = GetCurrentSize(static_cast<Handle>(m_handle_));
    return static_cast<T*>(GetElementAt(static_cast<Handle>(m_handle_), 0)) + current_size;
  }

 private:
  Handle m_handle_;
};

template<typename T>
TypedHandle<T> Create(size_t name_hash, size_t max_size, T* initial_data, size_t initial_count, ID3D11Device* device) {
  const auto& t_info = typeid(T);

  size_t type_hash = t_info.hash_code();
  size_t type_size = sizeof(T);
  size_t type_alignment = alignof(T);

  auto handle = Create(name_hash, type_hash, type_size, type_alignment, max_size, initial_data, initial_count, device);
  return TypedHandle<T>{ handle };
}

template<typename T>
TypedHandle<T> Create(const std::string& name, size_t max_size, T* initial_data, size_t initial_count, ID3D11Device* device) {
  std::hash<std::string> hasher;
  auto handle = Create(hasher(name), max_size, initial_data, initial_count, device);
  return TypedHandle<T>{ handle };
}

template<typename T>
T* GetCpuBuffer(TypedHandle<T> handle) {
  return static_cast<T*>(GetCpuBuffer(static_cast<Handle>(handle)));
}

template<typename T>
T* GetElementAt(TypedHandle<T> handle, size_t index) {
  return static_cast<T*>(GetElementAt(static_cast<Handle>(handle), index));
}

template<typename T>
ID3D11Buffer* GetGpuBuffer(TypedHandle<T> handle) {
  return GetGpuBuffer(static_cast<Handle>(handle));
}

template<typename T>
ID3D11Buffer** GetAddressOfGpuBuffer(TypedHandle<T> handle) {
  return GetAddressOfGpuBuffer(static_cast<Handle>(handle));
}

template<typename T>
ID3D11ShaderResourceView* GetShaderResourceView(TypedHandle<T> handle) {
  return GetShaderResourceView(static_cast<Handle>(handle));
}

template<typename T>
ID3D11ShaderResourceView** GetAddressOfShaderResourceView(TypedHandle<T> handle) {
  return GetAddressOfShaderResourceView(static_cast<Handle>(handle));
}

template<typename T>
void SetCurrentSize(TypedHandle<T> handle, size_t new_size) {
  SetCurrentSize(static_cast<Handle>(handle), new_size);
}

template<typename T>
size_t GetCurrentSize(TypedHandle<T> handle) {
  return GetCurrentSize(static_cast<Handle>(handle));
}

template<typename T>
bool Add(TypedHandle<T> handle, T* value) {
  return Add(static_cast<Handle>(handle), value);
}

template<typename T>
size_t GetMaxSize(TypedHandle<T> handle) {
  return GetMaxSize(static_cast<Handle>(handle));
}

template<typename T>
bool SendToGpu(TypedHandle<T> handle, ID3D11DeviceContext* device_context) {
  return SendToGpu(static_cast<Handle>(handle), device_context);
}

}  // namespace StructuredBuffer
}  // namespace Rendering