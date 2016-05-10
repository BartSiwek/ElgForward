#pragma once

#include <array>
#include <type_traits>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
#include <wrl.h>
#endif

#include "handle.h"

struct ResourceArrayTag {};

class ResourceArray {
public:
  using TagType = ResourceArrayTag;
  using HandleType = Handle<8, 24, TagType>;
  using Type = Microsoft::WRL::ComPtr<ID3D11InputLayout>;

  constexpr static const size_t Size = 255;  // Need a special terminator so 1 less

  ResourceArray() : m_array_(), m_freelist_head_(0) {
    for (size_t i = 0; i < Size; ++i) {
      m_array_[i].Generation = 0;
      m_array_[i].NextFreelistIndex = i + 1;
    }

    m_array_[Size-1].NextFreelistIndex = HandleType::MaxIndex;
  }

  ~ResourceArray() {
    size_t current_entry = m_freelist_head_;
    while (current_entry != HandleType::MaxIndex) {
      m_array_[current_entry].Generation = HandleType::MaxGenerarion;
      current_entry = m_array_[current_entry].NextFreelistIndex;
    }

    for (auto& entry : m_array_) {
      if (entry.Generation != HandleType::MaxGenerarion) {
        entry.Value.~Type();
        entry.Generation = HandleType::MaxGenerarion;
      }
    }
  }

  template<class... Types>
  HandleType Add(Types&&... args) {
    auto index = m_freelist_head_;
    if (index == HandleType::MaxIndex) {
      return {};
    }
    m_freelist_head_ = m_array_[index].NextFreelistIndex;

    auto ptr = std::addressof(m_array_[index].Value);
    ::new(ptr) Type(std::forward<Types>(args)...);
    
    auto generation = m_array_[index].Generation;

    return { index, generation };
  }

  bool IsActive(HandleType handle) {
    auto index = handle.Index;
    if (index < m_array_.size()) {
      return m_array_[index].Generation == handle.Generation;
    }
    return false;
  }

  Type& Get(HandleType handle) {
    return m_array_[handle.Index].Value;
  }

  const Type& Get(HandleType handle) const {
    return m_array_[handle.Index].Value;
  }

  void Remove(HandleType handle) {
    auto index = handle.Index;
    if (handle.Generation == m_array_[index].Generation) {
      m_array_[index].Value.~Type();
      m_array_[index].Generation += 1;
      m_array_[index].NextFreelistIndex = m_freelist_head_;
      m_freelist_head_ = index;
    }
  }

private:
  struct ArrayEntry {
    ArrayEntry() : Generation(0), NextFreelistIndex(0) {
      // Should be initialized by the ResourceArray
    }

    ~ArrayEntry() {
      // Should be cleaned up by the ResourceArray
    }

    HandleType::StorageType Generation;
    union {
      size_t NextFreelistIndex;
      Type Value;
    };
  };

  std::array<ArrayEntry, Size> m_array_;
  size_t m_freelist_head_;
};