#pragma once

#include <array>

#include <d3d11.h>
#include <wrl.h>

#include "com_helpers.h"
#include "mesh.h"

struct GpuMesh {
  static constexpr size_t VertexBufferCount = 3;

  static constexpr D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology = Mesh::PrimitiveTopology;

  GpuMesh() : IndexBuffer(nullptr) {
    for (size_t i = 0; i < VertexBufferCount; ++i) {
      VertexBuffers[i] = nullptr;
      VertexBufferStrides[i] = 0;
    }
  }

  ~GpuMesh() {
    for (size_t i = 0; i < VertexBufferCount; ++i) {
      SAFE_RELEASE(VertexBuffers[i]);
    }
  }

  GpuMesh(const GpuMesh&) = delete;
  GpuMesh& operator=(const GpuMesh&) = delete;

  GpuMesh(GpuMesh&&) = default;
  GpuMesh& operator=(GpuMesh&&) = default;

  std::array<ID3D11Buffer*, VertexBufferCount> VertexBuffers;
  std::array<uint32_t, VertexBufferCount> VertexBufferStrides;

  Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
  uint32_t IndexCount;
};
