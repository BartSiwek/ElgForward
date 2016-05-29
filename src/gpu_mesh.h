#pragma once

#include <vector>

#include <d3d11.h>

#include "filesystem.h"
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

struct GpuMeshTag {};

using GpuMeshHandle = Handle<8, 24, GpuMeshTag>;

struct MeshLoadOptions {
  DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R32_UINT;
};

bool CreateMesh(const std::string& prefix, const filesystem::path& path, const MeshLoadOptions& options, ID3D11Device* device, std::vector<GpuMeshHandle>* meshes);

GpuMesh* RetreiveMesh(GpuMeshHandle handle);
