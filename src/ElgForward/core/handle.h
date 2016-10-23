#pragma once

#include <cstdint>

namespace Core {

template<uint8_t I, uint8_t G, typename Tag>
class Handle {
 public:
  using StorageType = uint32_t;
  using TagType = Tag;

  constexpr static const StorageType MaxIndex = (1 << I) - 1;
  constexpr static const StorageType MaxGenerarion = (1 << G) - 1;

  Handle() : m_index_(MaxIndex), m_generation_(MaxGenerarion) {
  }

  Handle(StorageType index, StorageType generation) : m_index_(index), m_generation_(generation) {
  }

  StorageType GetIndex() {
    return m_index_;
  }

  void SetIndex(StorageType index) {
    m_index_ = index;
  }

  StorageType GetGeneration() {
    return m_generation_;
  }

  void SetGeneration(StorageType generation) {
    m_generation_ = generation;
  }

  void NextGeneration() {
    ++m_generation_;
  }

  bool IsValid() const {
    return m_index_ != MaxIndex && m_generation_ != MaxGenerarion;
  }

  StorageType CompactForm() {
    return (static_cast<StorageType>(m_index_) << G) | static_cast<StorageType>(m_generation_);
  }

 private:
  StorageType m_index_ : I;
  StorageType m_generation_ : G;

  static_assert(I + G <= 32, "The index and generation bit size exceeds 32 bits");
};

template<typename Tag>
class Handle<32, 0, Tag> {
 public:
  using StorageType = uint32_t;
  using TagType = Tag;

  constexpr static const StorageType MaxIndex = static_cast<uint32_t>(-1);
  constexpr static const StorageType MaxGenerarion = 0;

  Handle() : m_index_(MaxIndex) {
  }

  Handle(StorageType index) : m_index_(index) {
  }

  StorageType GetIndex() {
    return m_index_;
  }

  void SetIndex(StorageType index) {
    m_index_ = index;
  }

  StorageType CompactForm() {
    return m_index_;
  }

 private:
  StorageType m_index_;
};

}  // namespace Core
