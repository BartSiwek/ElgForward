#pragma once

#include <vector>

#include <d3d11.h>

#include "vertex_data.h"

class Mesh {
public:
  static constexpr uint32_t PositionInputSlot = 0;
  static constexpr uint32_t NormalInputSlot = 1;
  static constexpr uint32_t Texture0InputSlot = 0;

  static constexpr D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

  using PositionType = VertexDataPoisition<PositionInputSlot>;
  using NormalType = VertexDataNormal<NormalInputSlot>;
  using TextureType = VertexDataTexture<0, Texture0InputSlot>;

  Mesh() = default;
  ~Mesh() = default;

  Mesh(size_t expected_vertex_count, size_t expected_index_count) : Mesh() {
    Reserve(expected_vertex_count, expected_index_count);
  }

  Mesh(const Mesh& other) = delete;
  Mesh& operator=(const Mesh& other) = delete;

  Mesh(Mesh&& other) = default;
  Mesh& operator=(Mesh&& other) = default;

  void Reserve(size_t expected_vertex_count, size_t expected_index_count) {
    Positions.reserve(expected_vertex_count);
    Normals.reserve(expected_vertex_count);
    TextureCoords.reserve(expected_vertex_count);
    Indices.reserve(expected_index_count);
  }

  std::vector<PositionType> Positions;
  std::vector<NormalType> Normals;
  std::vector<TextureType> TextureCoords;
  std::vector<uint32_t> Indices;
};
