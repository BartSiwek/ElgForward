#pragma once

#include <cstdint>

namespace Rendering {

using VertexDataChannelsType = uint32_t;

const size_t MAX_TEXCOORDS = 8;

const size_t MAX_COLORS = 8;

enum class VertexDataChannel : VertexDataChannelsType {
  UNKNOWN = 0x0,
  POSITIONS = 0x1,
  NORMALS = 0x2,
  TANGENTS = 0x4,
  BITANGENTS = 0x8,
  TEXCOORDS0 = 0x10,
  TEXCOORDS1 = 0x20,
  TEXCOORDS2 = 0x40,
  TEXCOORDS3 = 0x80,
  TEXCOORDS4 = 0x100,
  TEXCOORDS5 = 0x200,
  TEXCOORDS6 = 0x400,
  TEXCOORDS7 = 0x800,
  COLORS0 = 0x1000,
  COLORS1 = 0x2000,
  COLORS2 = 0x4000,
  COLORS3 = 0x8000,
  COLORS4 = 0x10000,
  COLORS5 = 0x20000,
  COLORS6 = 0x40000,
  COLORS7 = 0x80000,
};

inline constexpr VertexDataChannel GetTexCoordsChannel(size_t index) {
  return static_cast<VertexDataChannel>(static_cast<VertexDataChannelsType>(VertexDataChannel::TEXCOORDS0) << index);
}

inline constexpr VertexDataChannel GetColorsChannel(size_t index) {
  return static_cast<VertexDataChannel>(static_cast<VertexDataChannelsType>(VertexDataChannel::COLORS0) << index);
}

}  // namespace Rendering