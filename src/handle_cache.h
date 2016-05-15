#pragma once

#include <unordered_map>

#include "hash.h"

template<typename K, typename H>
class HandleCache {
public:
  using KeyType = K;
  using HandleType = H;

  template<typename F>
  HandleType GetOrAdd(const KeyType& key, const F& factory) {
    auto it = m_storage_.find(key);

    if (it != std::end(m_storage_)) {
      return it->second;
    }

    auto new_handle = factory();
    if (new_handle.IsValid()) {
      m_storage_.emplace(key, new_handle);
    }
    return new_handle;
  }

private:
  std::unordered_map<KeyType, HandleType, std::hash<KeyType>, std::equal_to<KeyType>> m_storage_;
};
