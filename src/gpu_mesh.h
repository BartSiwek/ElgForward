#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "mesh.h"

struct GpuMesh {
  using PositionType = Mesh::PositionType;
  using NormalType = Mesh::NormalType;
  using TextureType = Mesh::TextureType;

  Microsoft::WRL::ComPtr<ID3D11Buffer> position_vertex_buffer;
  Microsoft::WRL::ComPtr<ID3D11Buffer> normal_vertex_buffer;
  Microsoft::WRL::ComPtr<ID3D11Buffer> texture_vertex_buffer;
  Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;
};

bool CreateGpuMesh(const Mesh& mesh, ID3D11Device* device, GpuMesh* gpu_mesh);
