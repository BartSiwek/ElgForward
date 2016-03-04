#pragma once

#include <d3d11.h>

#include "filesystem.h"
#include "gpu_mesh.h"

struct MeshLoaderOptions {
  DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R32_UINT;
};

bool LoadMesh(const filesystem::path& path, const MeshLoaderOptions& options, ID3D11Device* device, std::vector<GpuMesh>* meshes);
