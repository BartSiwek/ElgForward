#pragma once

template<uint8_t I, uint8_t G, typename Tag>
class Handle {
public:
  using StorageType = uint8_t;
  using TagType = Tag;

  void NextGeneration() {
    ++m_generation_;
  }

  uint32_t Index : I;
  uint32_t Generation : G;

  static_assert(I + G <= 32, "The index and generation bit size exceeds 32 bits");
};

template<typename Tag>
class Handle<32, 0, Tag> {
public:
  using StorageType = uint8_t;
  using TagType = Tag;

  uint32_t Index;
};
