#pragma once

#include <malloc.h>

#include <new>

#define PAD(n) char pad__LINE__[n]

template<typename T>
inline void* aligned_new(size_t size) {
  auto mem = _aligned_malloc(size, alignof(T));
  if (mem == nullptr) {
    throw std::bad_alloc();
  }
  return mem;
}

inline void aligned_delete(void* ptr) {
  _aligned_free(ptr);
}

template<typename T>
inline void* aligned_new_array(size_t size) {
  auto mem = _aligned_malloc(size, alignof(T));
  if (mem == nullptr) {
    throw std::bad_alloc();
  }
  return mem;
}

inline void aligned_delete_array(void* ptr) {
  _aligned_free(ptr);
}
