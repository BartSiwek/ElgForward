#pragma once

#include <vector>

#include <d3d11.h>

#include "filesystem.h"
#include "vertex_data.h"
#include "vertex_buffer.h"
#include "index_buffer.h"

struct Mesh {
  Mesh() = default;

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  Mesh(Mesh&&) = default;
  Mesh& operator=(Mesh&&) = default;

  std::vector<VertexBufferHandle> VertexBuffers = {};
  std::vector<DXGI_FORMAT> VertexBufferFormats = {};
  std::vector<uint32_t> VertexBufferStrides = {};
  std::vector<VertexDataChannel> VertexDataChannels = {};

  D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

  IndexBufferHandle IndexBuffer = {};
  DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_UNKNOWN;
  uint32_t IndexCount = 0;
};

struct MeshTag {};

using MeshHandle = Handle<8, 24, MeshTag>;

struct MeshLoadOptions {
  DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R32_UINT;
};

bool CreateMesh(const std::string& prefix, const filesystem::path& path, const MeshLoadOptions& options, ID3D11Device* device, std::vector<MeshHandle>* handles);

Mesh* RetreiveMesh(MeshHandle handle);
