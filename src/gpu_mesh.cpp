#include "gpu_mesh.h"

#include <fstream>
#include <memory>

#include <d3d11.h>
#include <DirectXMath.h>

#pragma warning(push)
#pragma warning(disable: 4201)
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma warning(pop)

#include <dxfw/dxfw.h>

#include "filesystem.h"
#include "hash.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "resource_array.h"
#include "handle_cache.h"

ResourceArray<GpuMeshHandle, std::unique_ptr<GpuMesh>, 255> g_storage_;
HandleCache<size_t, GpuMeshHandle> g_cache_;

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
bool AddVertexBufferToGpuMesh(size_t hash, const std::vector<T>& data, DXGI_FORMAT format, VertexDataChannel channel, ID3D11Device* device, GpuMesh* mesh) {
  auto vb_handle = CreateVertexBuffer(hash, data, device);
  if (!vb_handle.IsValid()) {
    return false;
  }

  mesh->VertexBuffers.emplace_back(vb_handle);
  mesh->VertexBufferFormats.emplace_back(format);
  mesh->VertexBufferStrides.emplace_back(sizeof(T));
  mesh->VertexDataChannels.emplace_back(channel);

  return true;
}

bool PrepareFloat1VertexBuffer(size_t base_hash, const aiVector3D* input, uint32_t vertex_count, VertexDataChannel channel, ID3D11Device* device, GpuMesh* mesh) {
  using EntryType = float;

  std::vector<EntryType> data(vertex_count);
  for (uint32_t i = 0; i < vertex_count; ++i) {
    data[i] = input[i].x;
  }

  size_t channel_hash = base_hash;
  hash_combine(channel_hash, channel);

  AddVertexBufferToGpuMesh(channel_hash, data, DXGI_FORMAT_R32_FLOAT, channel, device, mesh);

  return true;
}

bool PrepareFloat2VertexBuffer(size_t base_hash, const aiVector3D* input, uint32_t vertex_count, VertexDataChannel channel, ID3D11Device* device, GpuMesh* mesh) {
  using EntryType = DirectX::XMFLOAT2;

  std::vector<EntryType> data(vertex_count);
  for (uint32_t i = 0; i < vertex_count; ++i) {
    data[i].x = input[i].x;
    data[i].y = input[i].y;
  }

  size_t channel_hash = base_hash;
  hash_combine(channel_hash, channel);

  AddVertexBufferToGpuMesh(channel_hash, data, DXGI_FORMAT_R32G32_FLOAT, channel, device, mesh);

  return true;
}

bool PrepareFloat3VertexBuffer(size_t base_hash, const aiVector3D* input, uint32_t vertex_count, VertexDataChannel channel, ID3D11Device* device, GpuMesh* mesh) {
  using EntryType = DirectX::XMFLOAT3;

  std::vector<EntryType> data(vertex_count);
  for (uint32_t i = 0; i < vertex_count; ++i) {
    data[i].x = input[i].x;
    data[i].y = input[i].y;
    data[i].z = input[i].z;
  }

  size_t channel_hash = base_hash;
  hash_combine(channel_hash, channel);

  AddVertexBufferToGpuMesh(channel_hash, data, DXGI_FORMAT_R32G32B32_FLOAT, channel, device, mesh);

  return true;
}

bool PrepareFloat4VertexBuffer(size_t base_hash, const aiColor4D* input, uint32_t vertex_count, VertexDataChannel channel, ID3D11Device* device, GpuMesh* mesh) {
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

  AddVertexBufferToGpuMesh(channel_hash, data, DXGI_FORMAT_R32G32B32A32_FLOAT, channel, device, mesh);

  return true;
}

bool ValidateOptions(const MeshLoadOptions& options) {
  if (options.IndexBufferFormat != DXGI_FORMAT_R32_UINT && options.IndexBufferFormat != DXGI_FORMAT_R16_UINT) {
    return false;
  }

  return true;
}

bool LoadIndexBuffer32UInt(size_t hash, const aiMesh& imported_mesh, ID3D11Device* device, GpuMesh* mesh) {
  std::vector<uint32_t> indices;
  for (size_t face_index = 0; face_index < imported_mesh.mNumFaces; ++face_index) {
    for (size_t index_index = 0; index_index < imported_mesh.mFaces[face_index].mNumIndices; ++index_index) {
      indices.emplace_back(imported_mesh.mFaces[face_index].mIndices[index_index]);
    }
  }

  auto index_buffer_handle = CreateIndexBuffer(hash, indices, device);
  if (!index_buffer_handle.IsValid()) {
    return false;
  }

  mesh->IndexBuffer = index_buffer_handle;
  mesh->IndexBufferFormat = DXGI_FORMAT_R32_UINT;
  mesh->IndexCount = (uint32_t)indices.size();

  return true;
}

bool LoadIndexBuffer16UInt(size_t hash, const aiMesh& imported_mesh, ID3D11Device* device, GpuMesh* mesh) {
  std::vector<uint16_t> indices;
  for (size_t face_index = 0; face_index < imported_mesh.mNumFaces; ++face_index) {
    for (size_t index_index = 0; index_index < imported_mesh.mFaces[face_index].mNumIndices; ++index_index) {
      indices.emplace_back(static_cast<uint16_t>(imported_mesh.mFaces[face_index].mIndices[index_index]));
    }
  }

  auto index_buffer_handle = CreateIndexBuffer(hash, indices, device);
  if (!index_buffer_handle.IsValid()) {
    return false;
  }

  mesh->IndexBuffer = index_buffer_handle;
  mesh->IndexBufferFormat = DXGI_FORMAT_R16_UINT;
  mesh->IndexCount = (uint32_t)indices.size();

  return true;
}

bool CreateMesh(const std::string& prefix, const filesystem::path& path, const MeshLoadOptions& options, ID3D11Device* device, std::vector<GpuMeshHandle>* meshes) {
  if (meshes == nullptr) {
    DXFW_TRACE(__FILE__, __LINE__, true, "Got a null mesh vector pointer", nullptr);
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
    auto imported_mesh = scene->mMeshes[i];

    std::string mesh_name = prefix + ' ' + imported_mesh->mName.C_Str();
    size_t mesh_hash = std::hash<std::string>()(mesh_name);

    auto cached_handle = g_cache_.Get(mesh_hash);
    if (cached_handle.IsValid()) {
      meshes->emplace_back(cached_handle);
      continue;
    }

    if (imported_mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE) {
      DXFW_TRACE(__FILE__, __LINE__, true, "Only triangular meshes are supported for loading", nullptr);
      return false;
    }

    auto mesh = std::make_unique<GpuMesh>();

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

    auto new_mesh_handle = g_storage_.Add(std::move(mesh));
    g_cache_.Set(mesh_hash, new_mesh_handle);
    meshes->emplace_back(new_mesh_handle);
  }

  return true;
}

GpuMesh* RetreiveMesh(GpuMeshHandle handle) {
  return g_storage_.Get(handle).get();
}
