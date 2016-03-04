#pragma once

#include <array>

#include <d3d11.h>
#include <wrl.h>

#include "com_helpers.h"
#include "mesh.h"

struct GpuMesh {
  GpuMesh() = default;

  GpuMesh(const GpuMesh&) = delete;
  GpuMesh& operator=(const GpuMesh&) = delete;

  GpuMesh(GpuMesh&&) = default;
  GpuMesh& operator=(GpuMesh&&) = default;

  std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> VertexBuffers = {};
  std::vector<DXGI_FORMAT> VertexBufferFormats = {};
  std::vector<uint32_t> VertexBufferStrides = {};
  std::vector<VertexDataChannel> VertexDataChannels = {};

  D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

  Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer = nullptr;
  DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_UNKNOWN;
  uint32_t IndexCount = 0;
};

