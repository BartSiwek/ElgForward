#pragma once

#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include "mesh.h"

struct GpuMesh {
  using PositionType = Mesh::PositionType;
  using NormalType = Mesh::NormalType;
  using TextureType = Mesh::TextureType;

  static const size_t PositionVertexBufferIndex = 0;
  static const size_t NormalVertexBufferIndex = 1;
  static const size_t TextureCoordVertexBufferIndex = 2;

  static const size_t VertexBufferCount = 3;

  static bool Create(const Mesh& mesh, ID3D11Device* device, GpuMesh* gpu_mesh);

  static void InitializeLayout();

  std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> VertexBuffers;
  Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;

  static std::vector<D3D11_INPUT_ELEMENT_DESC> VetexBuffersLayout;
};
