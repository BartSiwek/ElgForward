#pragma once

#include <d3d11.h>

#include "filesystem.h"
#include "gpu_mesh.h"

bool LoadMesh(const filesystem::path& path, ID3D11Device* device, std::vector<GpuMesh>* meshes);
