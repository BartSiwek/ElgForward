#pragma once

#include <vector>

#include <d3d11.h>

#include "core/filesystem.h"
#include "vertex_data.h"
#include "rendering/vertex_buffer.h"
#include "rendering/index_buffer.h"

namespace Rendering {
namespace Mesh {

struct Mesh {
  Mesh() = default;
  ~Mesh() = default;

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  Mesh(Mesh&&) = default;
  Mesh& operator=(Mesh&&) = default;

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

Handle Create(size_t mesh_hash, std::unique_ptr<Mesh>&& data);

Handle Exists(size_t mesh_hash);

Mesh* Retreive(Handle handle);

}  // namespace Mesh
}  // namespace Rendering
