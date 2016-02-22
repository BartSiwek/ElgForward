#pragma once

#include <vector>

#include "vertex_data.h"

class Mesh {
public:
  static constexpr uint32_t PositionInputSlot = 0;
  static constexpr uint32_t NormalInputSlot = 1;
  static constexpr uint32_t Texture0InputSlot = 0;

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
    m_positions_.reserve(expected_vertex_count);
    m_normals_.reserve(expected_vertex_count);
    m_indices_.reserve(expected_index_count);
  }

  void AddVertex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT2& tex_coord) {
    m_positions_.emplace_back(pos);
    m_normals_.emplace_back(normal);
    m_texture_coords_.emplace_back(tex_coord);
  }

  void AddIndex(uint32_t index) {
    m_indices_.emplace_back(index);
  }

  const auto& Positions() const {
    return m_positions_;
  }

  const auto& Normals() const {
    return m_normals_;
  }

  const auto& TextureCoords() const {
    return m_texture_coords_;
  }

  const auto& Indices() const {
    return m_indices_;
  }

private:
  std::vector<PositionType> m_positions_;
  std::vector<NormalType> m_normals_;
  std::vector<TextureType> m_texture_coords_;
  std::vector<uint32_t> m_indices_;
};
