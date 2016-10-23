#pragma once

#include <array>

#include "core/com_helpers.h"

namespace Core {

template<typename T, size_t Size>
class ComArray {
 public:
  using value_type = T*;

  ComArray() : m_array_() {
  }

  ComArray(const ComArray& other) : m_array_(other.m_array_) {
    InternalAddRef();
  }

  ComArray& operator=(const ComArray& other) {
    if (this != &other) {
      InternalRelease();
      m_array_ = other.m_array_;
      InternalAddRef();
    }
    return *this;
  }

  ComArray(ComArray&& other) : m_array_(std::move(other.m_array_)) {
    other.InternalClean();
  }

  ComArray& operator=(ComArray&& other) {
    if (this != &other) {
      InternalRelease();
      m_array_ = std::move(other.m_array_);
      other.InternalClean();
    }
    return *this;
  }

  ~ComArray() {
    InternalRelease();
  }

  const value_type& Get(size_t index) const {
    // TODO: Add asserts on index
    return m_array_[index];
  }

  void Set(size_t index, const value_type& value) {
    // TODO: Add asserts on index
    if (m_array_[index] != nullptr) {
      SAFE_RELEASE(m_array_[index]);
    }

    if (value != nullptr) {
      m_array_[index] = value;
      m_array_[index]->AddRef();
    }
  }

 private:
  void InternalClean() {
    for (size_t i = 0; i < Size; ++i) {
      m_array_[i] = nullptr;
    }
  }

  void InternalAddRef() {
     for (size_t i = 0; i < Size; ++i) {
       if (m_array_[i] != nullptr) {
         m_array_[i]->AddRef();
       }
     }
  }

  void InternalRelease() {
     for (size_t i = 0; i < Size; ++i) {
       SAFE_RELEASE(m_array_[i]);
     }
  }

  std::array<value_type, Size> m_array_ = { nullptr };
};

}  // namespace Core
