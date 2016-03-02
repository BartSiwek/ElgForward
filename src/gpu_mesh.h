#pragma once

#include <array>

#include <d3d11.h>
#include <wrl.h>

#include "com_helpers.h"
#include "mesh.h"

struct GpuMesh {
  static constexpr D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology = Mesh::PrimitiveTopology;

  GpuMesh() : VertexBuffers(), VertexBufferFormats(), VertexBufferStrides(), ChannelsMask(0), IndexBuffer(), IndexCount(0) {
  }

  ~GpuMesh() = default;

  GpuMesh(const GpuMesh&) = delete;
  GpuMesh& operator=(const GpuMesh&) = delete;

  GpuMesh(GpuMesh&&) = default;
  GpuMesh& operator=(GpuMesh&&) = default;

  std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> VertexBuffers;
  std::vector<DXGI_FORMAT> VertexBufferFormats;
  std::vector<uint32_t> VertexBufferStrides;
  VertexDataChannelsMask ChannelsMask;

  Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
  uint32_t IndexCount;
};
