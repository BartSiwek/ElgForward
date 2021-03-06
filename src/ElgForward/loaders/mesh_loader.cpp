#include "mesh_loader.h"

#include <fstream>
#include <memory>

#include <d3d11.h>
#include <DirectXMath.h>

#pragma warning(push)
#pragma warning(disable: 4201)
#pragma warning(disable: 4305)
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4706)
#include <nlohmann/json.hpp>
#pragma warning(pop)

#include <dxfw/dxfw.h>

#include "core/filesystem.h"
#include "core/hash.h"
#include "core/resource_array.h"
#include "core/handle_cache.h"
#include "rendering/vertex_buffer.h"
#include "rendering/index_buffer.h"

using namespace Rendering;

namespace Loaders {

struct MeshLoadOptions {
  DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R32_UINT;
};

class AiLogStreamGuard {
public:
  AiLogStreamGuard(const aiLogStream& stream) : m_stream_(stream) {
    aiAttachLogStream(&m_stream_);
  }

  AiLogStreamGuard(aiLogStream&& stream) : m_stream_(std::move(stream)) {
    aiAttachLogStream(&m_stream_);
  }

  AiLogStreamGuard(const AiLogStreamGuard& stream) = delete;
  AiLogStreamGuard& operator=(const AiLogStreamGuard& stream) = delete;

  AiLogStreamGuard(AiLogStreamGuard&& stream) = delete;
  AiLogStreamGuard& operator=(AiLogStreamGuard&& stream) = delete;

  ~AiLogStreamGuard() {
    aiDetachLogStream(&m_stream_);
  }

private:
  aiLogStream m_stream_;
};

class AiSceneDeleter {
public:
  void operator()(const aiScene* scene) const {
    aiReleaseImport(scene);
  }
};

template<typename T>
bool AddVertexBufferToMesh(size_t hash, const std::vector<T>& data, DXGI_FORMAT format, VertexDataChannel channel, ID3D11Device* device, Mesh::Mesh* mesh) {
  auto vb_handle = Rendering::VertexBuffer::Create(hash, data, device);
  if (!vb_handle.IsValid()) {
    return false;
  }

  mesh->VertexBuffers.emplace_back(vb_handle);
  mesh->VertexBufferFormats.emplace_back(format);
  mesh->VertexBufferStrides.emplace_back(static_cast<uint32_t>(sizeof(T)));
  mesh->VertexDataChannels.emplace_back(channel);

  return true;
}

bool PrepareFloat1VertexBuffer(size_t base_hash, const aiVector3D* input, uint32_t vertex_count, VertexDataChannel channel, ID3D11Device* device, Mesh::Mesh* mesh) {
  using EntryType = float;

  std::vector<EntryType> data(vertex_count);
  for (uint32_t i = 0; i < vertex_count; ++i) {
    data[i] = input[i].x;
  }

  size_t channel_hash = base_hash;
  hash_combine(channel_hash, channel);

  AddVertexBufferToMesh(channel_hash, data, DXGI_FORMAT_R32_FLOAT, channel, device, mesh);

  return true;
}

bool PrepareFloat2VertexBuffer(size_t base_hash, const aiVector3D* input, uint32_t vertex_count, VertexDataChannel channel, ID3D11Device* device, Mesh::Mesh* mesh) {
  using EntryType = DirectX::XMFLOAT2;

  std::vector<EntryType> data(vertex_count);
  for (uint32_t i = 0; i < vertex_count; ++i) {
    data[i].x = input[i].x;
    data[i].y = input[i].y;
  }

  size_t channel_hash = base_hash;
  hash_combine(channel_hash, channel);

  AddVertexBufferToMesh(channel_hash, data, DXGI_FORMAT_R32G32_FLOAT, channel, device, mesh);

  return true;
}

bool PrepareFloat3VertexBuffer(size_t base_hash, const aiVector3D* input, uint32_t vertex_count, VertexDataChannel channel, ID3D11Device* device, Mesh::Mesh* mesh) {
  using EntryType = DirectX::XMFLOAT3;

  std::vector<EntryType> data(vertex_count);
  for (uint32_t i = 0; i < vertex_count; ++i) {
    data[i].x = input[i].x;
    data[i].y = input[i].y;
    data[i].z = input[i].z;
  }

  size_t channel_hash = base_hash;
  hash_combine(channel_hash, channel);

  AddVertexBufferToMesh(channel_hash, data, DXGI_FORMAT_R32G32B32_FLOAT, channel, device, mesh);

  return true;
}

bool PrepareFloat4VertexBuffer(size_t base_hash, const aiColor4D* input, uint32_t vertex_count, VertexDataChannel channel, ID3D11Device* device, Mesh::Mesh* mesh) {
  using EntryType = DirectX::XMFLOAT4;

  std::vector<EntryType> data(vertex_count);
  for (uint32_t i = 0; i < vertex_count; ++i) {
    data[i].x = input[i].r;
    data[i].y = input[i].g;
    data[i].z = input[i].b;
    data[i].w = input[i].a;
  }

  size_t channel_hash = base_hash;
  hash_combine(channel_hash, channel);

  AddVertexBufferToMesh(channel_hash, data, DXGI_FORMAT_R32G32B32A32_FLOAT, channel, device, mesh);

  return true;
}

bool ReadOptions(const nlohmann::json& json_options, MeshLoadOptions* options) {
  bool are_valid_options = json_options["index_buffer_format"].is_string();
  if (!are_valid_options) {
    return false;
  }

  const std::string& index_buffer_format = json_options["index_buffer_format"];
  if (index_buffer_format == "32_UINT") {
    options->IndexBufferFormat = DXGI_FORMAT_R32_UINT;
    return true;
  } else if (index_buffer_format == "16_UINT") {
    options->IndexBufferFormat = DXGI_FORMAT_R16_UINT;
    return true;
  } else {
    return false;
  }
}

bool ValidateOptions(const MeshLoadOptions& options) {
  if (options.IndexBufferFormat != DXGI_FORMAT_R32_UINT && options.IndexBufferFormat != DXGI_FORMAT_R16_UINT) {
    return false;
  }

  return true;
}

bool LoadIndexBuffer32UInt(size_t hash, const aiMesh& imported_mesh, ID3D11Device* device, Mesh::Mesh* mesh) {
  std::vector<uint32_t> indices;
  for (size_t face_index = 0; face_index < imported_mesh.mNumFaces; ++face_index) {
    for (size_t index_index = 0; index_index < imported_mesh.mFaces[face_index].mNumIndices; ++index_index) {
      indices.emplace_back(imported_mesh.mFaces[face_index].mIndices[index_index]);
    }
  }

  auto index_buffer_handle = Rendering::IndexBuffer::Create(hash, indices, device);
  if (!index_buffer_handle.IsValid()) {
    return false;
  }

  mesh->IndexBuffer = index_buffer_handle;
  mesh->IndexBufferFormat = DXGI_FORMAT_R32_UINT;
  mesh->IndexCount = (uint32_t)indices.size();

  return true;
}

bool LoadIndexBuffer16UInt(size_t hash, const aiMesh& imported_mesh, ID3D11Device* device, Mesh::Mesh* mesh) {
  std::vector<uint16_t> indices;
  for (size_t face_index = 0; face_index < imported_mesh.mNumFaces; ++face_index) {
    for (size_t index_index = 0; index_index < imported_mesh.mFaces[face_index].mNumIndices; ++index_index) {
      indices.emplace_back(static_cast<uint16_t>(imported_mesh.mFaces[face_index].mIndices[index_index]));
    }
  }

  auto index_buffer_handle = Rendering::IndexBuffer::Create(hash, indices, device);
  if (!index_buffer_handle.IsValid()) {
    return false;
  }

  mesh->IndexBuffer = index_buffer_handle;
  mesh->IndexBufferFormat = DXGI_FORMAT_R16_UINT;
  mesh->IndexCount = (uint32_t)indices.size();

  return true;
}

bool ReadMeshes(const std::string& prefix, const filesystem::path& path, const MeshLoadOptions& options, ID3D11Device* device, std::vector<MeshIdentifier>* identifiers) {
  if (identifiers == nullptr) {
    DXFW_TRACE(__FILE__, __LINE__, true, "Got a null identifier vector pointer", nullptr);
    return false;
  }

  bool validate_ok = ValidateOptions(options);
  if (!validate_ok) {
    DXFW_TRACE(__FILE__, __LINE__, true, "Invalid mesh loading options", nullptr);
    return false;
  }

  AiLogStreamGuard log_stream_guard(aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr));

  std::string path_string = path.string();
  std::unique_ptr<const aiScene, AiSceneDeleter> scene(aiImportFile(path_string.c_str(), aiProcessPreset_TargetRealtime_MaxQuality));

  if (!scene->HasMeshes()) {
    return false;
  }

  for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
    auto& imported_mesh = scene->mMeshes[i];

    std::string mesh_name = prefix + ' ' + imported_mesh->mName.C_Str();
    size_t mesh_hash = std::hash<std::string>()(mesh_name);

    auto cached_handle = Mesh::Exists(mesh_hash);
    if (cached_handle.IsValid()) {
      identifiers->emplace_back(MeshIdentifier{ mesh_hash, cached_handle });
      continue;
    }

    if (imported_mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE) {
      DXFW_TRACE(__FILE__, __LINE__, true, "Only triangular meshes are supported for loading", nullptr);
      return false;
    }

    auto mesh = std::make_unique<Mesh::Mesh>();

    uint32_t vertex_count = imported_mesh->mNumVertices;

    if (imported_mesh->HasPositions()) {
      bool prep_ok = PrepareFloat3VertexBuffer(mesh_hash, imported_mesh->mVertices, vertex_count, VertexDataChannel::POSITIONS, device, mesh.get());
      if (!prep_ok) {
        return false;
      }
    }

    if (imported_mesh->HasNormals()) {
      bool prep_ok = PrepareFloat3VertexBuffer(mesh_hash, imported_mesh->mNormals, vertex_count, VertexDataChannel::NORMALS, device, mesh.get());
      if (!prep_ok) {
        return false;
      }
    }

    if (imported_mesh->HasTangentsAndBitangents()) {
      bool tangent_prep_ok = PrepareFloat3VertexBuffer(mesh_hash, imported_mesh->mTangents, vertex_count, VertexDataChannel::TANGENTS, device, mesh.get());
      if (!tangent_prep_ok) {
        return false;
      }

      bool bitangent_prep_ok = PrepareFloat3VertexBuffer(mesh_hash, imported_mesh->mBitangents, vertex_count, VertexDataChannel::BITANGENTS, device, mesh.get());
      if (!bitangent_prep_ok) {
        return false;
      }
    }

    static_assert(MAX_TEXCOORDS <= AI_MAX_NUMBER_OF_TEXTURECOORDS, "MAX_TEXCOORDS must be no more than AI_MAX_NUMBER_OF_TEXTURECOORDS");
    for (size_t texture_index = 0; texture_index < MAX_TEXCOORDS; ++texture_index) {
      if (imported_mesh->mTextureCoords[texture_index] != nullptr) {
        if (imported_mesh->mNumUVComponents[texture_index] == 1) {
          PrepareFloat1VertexBuffer(mesh_hash, imported_mesh->mTextureCoords[texture_index], vertex_count, GetTexCoordsChannel(texture_index), device, mesh.get());
        } else if (imported_mesh->mNumUVComponents[texture_index] == 2) {
          PrepareFloat2VertexBuffer(mesh_hash, imported_mesh->mTextureCoords[texture_index], vertex_count, GetTexCoordsChannel(texture_index), device, mesh.get());
        } else {  // imported_mesh->mNumUVComponents[texture_index] == 3
          PrepareFloat3VertexBuffer(mesh_hash, imported_mesh->mTextureCoords[texture_index], vertex_count, GetTexCoordsChannel(texture_index), device, mesh.get());
        }
      }
    }

    static_assert(MAX_COLORS <= AI_MAX_NUMBER_OF_COLOR_SETS, "MAX_COLORS must be no more than AI_MAX_NUMBER_OF_COLOR_SETS");
    for (size_t color_index = 0; color_index < MAX_COLORS; ++color_index) {
      if (imported_mesh->mColors[color_index] != nullptr) {
        bool prep_ok = PrepareFloat4VertexBuffer(mesh_hash, imported_mesh->mColors[color_index], vertex_count, GetColorsChannel(color_index), device, mesh.get());
        if (!prep_ok) {
          return false;
        }
      }
    }

    mesh->PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    if (options.IndexBufferFormat == DXGI_FORMAT_R32_UINT) {
      bool indices_ok = LoadIndexBuffer32UInt(mesh_hash, *imported_mesh, device, mesh.get());
      if (!indices_ok) {
        return false;
      }
    } else if (options.IndexBufferFormat == DXGI_FORMAT_R16_UINT) {
      bool indices_ok = LoadIndexBuffer16UInt(mesh_hash, *imported_mesh, device, mesh.get());
      if (!indices_ok) {
        return false;
      }
    } else {
      return false;
    }

    auto new_mesh_handle = Mesh::Create(mesh_hash, std::move(mesh));
    identifiers->emplace_back(MeshIdentifier{ mesh_hash, new_mesh_handle });
  }

  return true;
}

bool ReadMesh(const nlohmann::json& json_mesh, const filesystem::path& base_path, ID3D11Device* device, std::vector<MeshIdentifier>* mesh_identifiers) {
  const auto& json_options = json_mesh["options"];

  MeshLoadOptions options;
  bool options_ok = ReadOptions(json_options, &options);
  if (!options_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid mesh options %S", json_options.dump().c_str());
    return false;
  }

  const std::string& prefix = json_mesh["prefix"];
  const std::string& path = json_mesh["path"];

  auto meshes_path = base_path / path;

  bool meshes_ok = ReadMeshes(prefix, meshes_path, options, device, mesh_identifiers);
  if (!meshes_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error loading meshes from %S", meshes_path.string().c_str());
    return false;
  }

  return true;
}

}  // namespace Loaders
