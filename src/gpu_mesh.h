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

  static const uint32_t InputLayoutElementCount = PositionType::InputLayoutElementCount
    + NormalType::InputLayoutElementCount
    + TextureType::InputLayoutElementCount;
  static const std::array<D3D11_INPUT_ELEMENT_DESC, InputLayoutElementCount> InputLayout;

  std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> VertexBuffers;
  Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
};

bool CreateGpuMesh(const Mesh& mesh, ID3D11Device* device, GpuMesh* gpu_mesh);
