#pragma once

#include <unordered_map>

#include "hash.h"

namespace Core {

template<typename K, typename H>
class HandleCache {
public:
  using KeyType = K;
  using HandleType = H;

  HandleType Get(const KeyType& key) const {
    auto it = m_storage_.find(key);

    if (it != std::end(m_storage_)) {
      return it->second;
    }

    return {};
  }

  void Set(const KeyType& key, HandleType handle) {
    m_storage_[key] = handle;
  }

private:
  std::unordered_map<KeyType, HandleType, std::hash<KeyType>, std::equal_to<KeyType>> m_storage_;
};

}  // namespace Core
