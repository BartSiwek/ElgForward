#pragma once

#include <unordered_map>

#include "hash.h"

template<typename KeyType, typename HandleType>
class HandleCache {
public:
  using Key = KeyType;
  using Value = HandleType;

  template<typename F>
  Value& Get(const Key& key, const F& factory) {
    auto it = m_storage_.find(key);

    if (it != std::end(m_storage_)) {
      return it->second;
    }

    auto result = m_storage_.emplace(key, factory());
    return result.first->second;
  }

private:
  std::unordered_map<KeyType, HandleType, std::hash<KeyType>, std::equal_to<KeyType>> m_storage_;
};
