#pragma once

#include "filesystem.h"
#include "mesh.h"

bool LoadMesh(const filesystem::path& path, std::vector<Mesh>* meshes);
