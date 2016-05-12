#pragma once

/*
 * taken from Boost.functional hash implementation.
 * See https://github.com/boostorg/functional for code.
 * See http://www.boost.org/LICENSE_1_0.txt for license.
 */

#include <functional>

namespace hash_detail {

template <typename SizeT>
inline void hash_combine_impl(SizeT& seed, SizeT value) {
  seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

inline void hash_combine_impl(uint32_t& h1, uint32_t k1) {
  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;

  k1 *= c1;
  k1 = _rotl(k1, 15);
  k1 *= c2;

  h1 ^= k1;
  h1 = _rotl(h1, 13);
  h1 = h1 * 5 + 0xe6546b64;
}

inline void hash_combine_impl(uint64_t& h, uint64_t k) {
  const uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
  const int r = 47;

  k *= m;
  k ^= k >> r;
  k *= m;

  h ^= k;
  h *= m;

  // Completely arbitrary number, to prevent 0's
  // from hashing to 0.
  h += 0xe6546b64;
}

}  // hash_detail

template<class T>
inline void hash_combine(std::size_t& seed, const T& v) {
  std::hash<T> hasher;
  return hash_detail::hash_combine_impl(seed, hasher(v));
}

template<class It>
inline size_t hash_range(It first, It last) {
  size_t seed = 0;

  for (; first != last; ++first) {
    hash_combine(seed, *first);
  }

  return seed;
}

template<class It>
inline void hash_range(std::size_t& seed, It first, It last) {
  for (; first != last; ++first) {
    hash_combine(seed, *first);
  }
}
