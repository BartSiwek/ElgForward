#pragma once

#include <vector>

#include <d3d11.h>

#include "core/filesystem.h"
#include "vertex_data.h"
#include "rendering/vertex_buffer.h"
#include "rendering/index_buffer.h"

namespace Rendering {
namespace Mesh {

struct MeshData {
  MeshData() = default;
  ~MeshData() = default;

  MeshData(const MeshData&) = delete;
  MeshData& operator=(const MeshData&) = delete;

  MeshData(MeshData&&) = default;
  MeshData& operator=(MeshData&&) = default;

  std::vector<Rendering::VertexBuffer::Handle> VertexBuffers = {};
  std::vector<DXGI_FORMAT> VertexBufferFormats = {};
  std::vector<uint32_t> VertexBufferStrides = {};
  std::vector<VertexDataChannel> VertexDataChannels = {};

  D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

  Rendering::IndexBuffer::Handle IndexBuffer = {};
  DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_UNKNOWN;
  uint32_t IndexCount = 0;
};

struct MeshTag {};

using Handle = Core::Handle<8, 24, MeshTag>;

Handle Create(size_t mesh_hash, std::unique_ptr<MeshData>&& data);

Handle Exists(size_t mesh_hash);

MeshData* Retreive(Handle handle);

}  // namespace Mesh
}  // namespace Rendering
