#pragma once

#include <vector>

#include <d3d11.h>

#include "vertex_data.h"
#include "vertex_buffer.h"
#include "index_buffer.h"

struct GpuMesh {
  GpuMesh() = default;

  GpuMesh(const GpuMesh&) = delete;
  GpuMesh& operator=(const GpuMesh&) = delete;

  GpuMesh(GpuMesh&&) = default;
  GpuMesh& operator=(GpuMesh&&) = default;

  std::vector<VertexBufferHandle> VertexBuffers = {};
  std::vector<DXGI_FORMAT> VertexBufferFormats = {};
  std::vector<uint32_t> VertexBufferStrides = {};
  std::vector<VertexDataChannel> VertexDataChannels = {};

  D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

  IndexBufferHandle IndexBuffer = {};
  DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_UNKNOWN;
  uint32_t IndexCount = 0;
};

