#pragma once

#include <vector>

#include <d3d11.h>

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "rendering/mesh.h"

namespace Loaders {

struct MeshIdentifier {
  size_t Hash;
  Rendering::Mesh::Handle handle;
};

bool ReadMesh(const nlohmann::json& json_mesh, const filesystem::path& base_path, ID3D11Device* device, std::vector<MeshIdentifier>* mesh_identifiers);

}  // namespace Loaders
