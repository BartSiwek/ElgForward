#pragma once

#include <array>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
// #include <Windows.h>
// #include <dxgi.h>
#include <d3d11.h>
#include <wrl.h>
#endif

#include "handle.h"

struct ResourceArrayTag {};

class FreelistArray {
public:
  using Type = Microsoft::WRL::ComPtr<ID3D11InputLayout>;

  constexpr static const size_t Size = 256;

  FreelistArray() : m_freelist_head_(0) {
    for (size_t i = 0; i < Size; ++i) {
      m_array_[i].NextFreelistIndex = i + 1;
    }
  }

  ~FreelistArray() {
    // TODO: Walk the array and remove everything not on the freelist
  }

private:
  struct ArrayEntry {
    ArrayEntry() : NextFreelistIndex(0) {
      // Should be initialized by the FreelistArray
    }

    ~ArrayEntry() {
      // Should be cleaned up by the FreelistArray
    }

    union {
      size_t NextFreelistIndex;
      Type Value;
    };
  };

  std::array<ArrayEntry, Size> m_array_;
  size_t m_freelist_head_;
};

class ResourceArray {
public:
  using TagType = ResourceArrayTag;
  using HandleType = Handle<8, 24, TagType>;
  using Type = FreelistArray::Type;

  constexpr static const size_t Size = FreelistArray::Size;

  ResourceArray() : m_generations_(), m_array_() {
    for (size_t i = 0; i < Size; ++i) {
      m_generations_[i] = 0;
    }
  }

  ~ResourceArray() {
  }

  HandleType Add(const Type&) {
    return {};
  }

  bool IsActive(HandleType) {
    return false;
  }

  void Remove(HandleType) {

  }

private:
  std::array<HandleType::StorageType, Size> m_generations_;
  FreelistArray m_array_;
};