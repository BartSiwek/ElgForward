#pragma once

#include <array>

#include <DirectXMath.h>
#include <d3d11.h>

using VertexDataChannelsMask = uint32_t;

const size_t MAX_TEXCOORDS = 8;

const size_t MAX_COLORS = 8;

enum class VertexDataChannel : VertexDataChannelsMask {
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

inline VertexDataChannelsMask& operator|=(VertexDataChannelsMask& lhs, VertexDataChannel rhs) {
  lhs = lhs | static_cast<VertexDataChannelsMask>(rhs);
  return lhs;
}

inline constexpr VertexDataChannel GetTexCoordsChannel(size_t index) {
  return static_cast<VertexDataChannel>(static_cast<VertexDataChannelsMask>(VertexDataChannel::TEXCOORDS0) << index);
}

inline constexpr VertexDataChannel GetColorsChannel(size_t index) {
  return static_cast<VertexDataChannel>(static_cast<VertexDataChannelsMask>(VertexDataChannel::COLORS0) << index);
}

template<uint32_t InputSlot>
struct VertexDataPoisition {
  VertexDataPoisition() = default;
  ~VertexDataPoisition() = default;

  VertexDataPoisition(float x, float y, float z) : Position(x, y, z) {
  }

  VertexDataPoisition(const DirectX::XMFLOAT3& position) : Position(position) {
  }

  VertexDataPoisition(const VertexDataPoisition&) = default;
  VertexDataPoisition& operator=(const VertexDataPoisition&) = default;

  VertexDataPoisition(VertexDataPoisition&&) = default;
  VertexDataPoisition& operator=(VertexDataPoisition&&) = default;

  static const uint32_t InputLayoutElementCount = 1;
  static const std::array<D3D11_INPUT_ELEMENT_DESC, InputLayoutElementCount> InputLayout;

  DirectX::XMFLOAT3 Position;
};

static_assert(sizeof(VertexDataPoisition<0>) == sizeof(DirectX::XMFLOAT3), "Invalid VertexDataPoisition size");
static_assert(alignof(VertexDataPoisition<0>) == alignof(DirectX::XMFLOAT3), "Invalid VertexDataPoisition alignment");

template<uint32_t InputSlot>
const std::array<D3D11_INPUT_ELEMENT_DESC, VertexDataPoisition<InputSlot>::InputLayoutElementCount> VertexDataPoisition<InputSlot>::InputLayout = {
  { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, InputSlot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

template<uint32_t InputSlot>
struct VertexDataNormal {
  VertexDataNormal() = default;
  ~VertexDataNormal() = default;

  VertexDataNormal(float x, float y, float z) : Normal(x, y, z) {
  }

  VertexDataNormal(const DirectX::XMFLOAT3& normal) : Normal(normal) {
  }

  VertexDataNormal(const VertexDataNormal&) = default;
  VertexDataNormal& operator=(const VertexDataNormal&) = default;

  VertexDataNormal(VertexDataNormal&&) = default;
  VertexDataNormal& operator=(VertexDataNormal&&) = default;

  static const uint32_t InputLayoutElementCount = 1;
  static const std::array<D3D11_INPUT_ELEMENT_DESC, InputLayoutElementCount> InputLayout;

  DirectX::XMFLOAT3 Normal;
};

static_assert(sizeof(VertexDataNormal<0>) == sizeof(DirectX::XMFLOAT3), "Invalid VertexDataNormal size");
static_assert(alignof(VertexDataNormal<0>) == alignof(DirectX::XMFLOAT3), "Invalid VertexDataNormal alignment");

template<uint32_t InputSlot>
const std::array<D3D11_INPUT_ELEMENT_DESC, VertexDataNormal<InputSlot>::InputLayoutElementCount> VertexDataNormal<InputSlot>::InputLayout = {
  { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, InputSlot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

template<uint32_t Index, uint32_t InputSlot>
struct VertexDataTexture {
  VertexDataTexture() = default;
  ~VertexDataTexture() = default;

  VertexDataTexture(float u, float v) : TextureCoords(u, v) {
  }

  VertexDataTexture(const DirectX::XMFLOAT2& tex_coords) : TextureCoords(tex_coords) {
  }

  VertexDataTexture(const VertexDataTexture&) = default;
  VertexDataTexture& operator=(const VertexDataTexture&) = default;

  VertexDataTexture(VertexDataTexture&&) = default;
  VertexDataTexture& operator=(VertexDataTexture&&) = default;

  static const uint32_t InputLayoutElementCount = 1;
  static const std::array<D3D11_INPUT_ELEMENT_DESC, InputLayoutElementCount> InputLayout;

  DirectX::XMFLOAT2 TextureCoords;
};

static_assert(sizeof(VertexDataTexture<0, 0>) == sizeof(DirectX::XMFLOAT2), "Invalid VertexDataTexture size");
static_assert(alignof(VertexDataTexture<0, 0>) == alignof(DirectX::XMFLOAT2), "Invalid VertexDataTexture alignment");

template<uint32_t Index, uint32_t InputSlot>
const std::array<D3D11_INPUT_ELEMENT_DESC, VertexDataTexture<Index, InputSlot>::InputLayoutElementCount> VertexDataTexture<Index, InputSlot>::InputLayout = {
  { "TEXCOORD", Index, DXGI_FORMAT_R32G32B32_FLOAT, InputSlot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
