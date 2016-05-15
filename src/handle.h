#pragma once

template<uint8_t I, uint8_t G, typename Tag>
class Handle {
public:
  using StorageType = uint32_t;
  using TagType = Tag;

  constexpr static const StorageType MaxIndex = (1 << I) - 1;
  constexpr static const StorageType MaxGenerarion = (1 << G) - 1;

  Handle() : Index(MaxIndex), Generation(MaxGenerarion) {
  }

  Handle(StorageType index, StorageType generation) : Index(index), Generation(generation) {
  }

  void NextGeneration() {
    ++Generation;
  }

  bool IsValid() {
    return Index != MaxIndex && Generation != MaxGenerarion;
  }

  StorageType CompactForm() {
    return (static_cast<StorageType>(Index) << G) | static_cast<StorageType>(Generation);
  }

  StorageType Index : I;
  StorageType Generation : G;

  static_assert(I + G <= 32, "The index and generation bit size exceeds 32 bits");
};

template<typename Tag>
class Handle<32, 0, Tag> {
public:
  using StorageType = uint32_t;
  using TagType = Tag;

  constexpr static const StorageType MaxIndex = static_cast<uint32_t>(-1);
  constexpr static const StorageType MaxGenerarion = 0;

  Handle() : Index(MaxIndex) {
  }

  Handle(StorageType index) : Index(index) {
  }

  StorageType CompactForm() {
    return Index;
  }

  StorageType Index;
};
