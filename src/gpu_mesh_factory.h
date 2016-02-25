#pragma once

#include <array>

#include <d3d11.h>
#include <wrl.h>

#include "gpu_mesh.h"
#include "mesh.h"

struct GpuMeshFactory {
  static const size_t PositionVertexBufferIndex = 0;
  static const size_t NormalVertexBufferIndex = 1;
  static const size_t TextureCoordVertexBufferIndex = 2;

  static const uint32_t InputLayoutElementCount = Mesh::PositionType::InputLayoutElementCount
                                                + Mesh::NormalType::InputLayoutElementCount
                                                + Mesh::TextureType::InputLayoutElementCount;
  static const std::array<D3D11_INPUT_ELEMENT_DESC, InputLayoutElementCount> InputLayout;

  static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R32_UINT;  // Can otherwise be DXGI_FORMAT_R16_UINT

  static bool CreateGpuMesh(const Mesh& mesh, ID3D11Device* device, GpuMesh* gpu_mesh);
};

