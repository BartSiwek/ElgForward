#include "mesh_loader.h"

#include <fstream>
#include <memory>

#include <d3d11.h>

#pragma warning(push)
#pragma warning(disable: 4201)
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma warning(pop)

#include <dxfw/dxfw.h>

#include "filesystem.h"

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

template<typename EntryType>
bool CreateVertexBuffer(const std::vector<EntryType>& data, ID3D11Device* device, ID3D11Buffer** buffer) {
  D3D11_BUFFER_DESC bufferDesc;
  ZeroMemory(&bufferDesc, sizeof(bufferDesc));

  bufferDesc.Usage = D3D11_USAGE_DEFAULT;
  bufferDesc.ByteWidth = data.size() * sizeof(EntryType);
  bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bufferDesc.CPUAccessFlags = 0;
  bufferDesc.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA bufferData;
  ZeroMemory(&bufferData, sizeof(bufferData));
  bufferData.pSysMem = &data[0];

  HRESULT create_buffer_result = device->CreateBuffer(&bufferDesc, &bufferData, buffer);

  if (FAILED(create_buffer_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, create_buffer_result, true);
    return false;
  }

  return true;
}

template<typename IndexType>
bool CreateIndexBuffer(const std::vector<IndexType>& data, ID3D11Device* device, ID3D11Buffer** buffer) {
  D3D11_BUFFER_DESC bufferDesc;
  ZeroMemory(&bufferDesc, sizeof(bufferDesc));

  bufferDesc.Usage = D3D11_USAGE_DEFAULT;
  bufferDesc.ByteWidth = data.size() * sizeof(IndexType);
  bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bufferDesc.CPUAccessFlags = 0;
  bufferDesc.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA bufferData;
  ZeroMemory(&bufferData, sizeof(bufferData));
  bufferData.pSysMem = &data[0];

  HRESULT create_buffer_result = device->CreateBuffer(&bufferDesc, &bufferData, buffer);

  if (FAILED(create_buffer_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, create_buffer_result, true);
    return false;
  }

  return true;
}

template<typename T>
bool AddVertexBufferToGpuMesh(const std::vector<T>& data, DXGI_FORMAT format, VertexDataChannel channel, ID3D11Device* device, GpuMesh* mesh) {
  Microsoft::WRL::ComPtr<ID3D11Buffer> vb;
  bool vb_ok = CreateVertexBuffer(data, device, vb.GetAddressOf());
  if (!vb_ok) {
    return false;
  }

  mesh->VertexBuffers.emplace_back(vb);
  mesh->VertexBufferFormats.emplace_back(format);
  mesh->VertexBufferStrides.emplace_back(sizeof(T));
  mesh->VertexDataChannels.emplace_back(channel);

  return true;
}

bool PrepareFloat1VertexBuffer(const aiVector3D* input, uint32_t vertex_count, VertexDataChannel channel, ID3D11Device* device, GpuMesh* mesh) {
  using EntryType = float;

  std::vector<EntryType> data(vertex_count);
  for (uint32_t i = 0; i < vertex_count; ++i) {
    data[i] = input[i].x;
  }

  AddVertexBufferToGpuMesh(data, DXGI_FORMAT_R32_FLOAT, channel, device, mesh);

  return true;
}

bool PrepareFloat2VertexBuffer(const aiVector3D* input, uint32_t vertex_count, VertexDataChannel channel, ID3D11Device* device, GpuMesh* mesh) {
  using EntryType = DirectX::XMFLOAT2;

  std::vector<EntryType> data(vertex_count);
  for (uint32_t i = 0; i < vertex_count; ++i) {
    data[i].x = input[i].x;
    data[i].y = input[i].y;
  }

  AddVertexBufferToGpuMesh(data, DXGI_FORMAT_R32G32_FLOAT, channel, device, mesh);

  return true;
}

bool PrepareFloat3VertexBuffer(const aiVector3D* input, uint32_t vertex_count, VertexDataChannel channel, ID3D11Device* device, GpuMesh* mesh) {
  using EntryType = DirectX::XMFLOAT3;

  std::vector<EntryType> data(vertex_count);
  for (uint32_t i = 0; i < vertex_count; ++i) {
    data[i].x = input[i].x;
    data[i].y = input[i].y;
    data[i].z = input[i].z;
  }

  AddVertexBufferToGpuMesh(data, DXGI_FORMAT_R32G32B32_FLOAT, channel, device, mesh);

  return true;
}

bool PrepareFloat4VertexBuffer(const aiColor4D* input, uint32_t vertex_count, VertexDataChannel channel, ID3D11Device* device, GpuMesh* mesh) {
  using EntryType = DirectX::XMFLOAT4;

  std::vector<EntryType> data(vertex_count);
  for (uint32_t i = 0; i < vertex_count; ++i) {
    data[i].x = input[i].r;
    data[i].y = input[i].g;
    data[i].z = input[i].b;
    data[i].w = input[i].a;
  }

  AddVertexBufferToGpuMesh(data, DXGI_FORMAT_R32G32B32A32_FLOAT, channel, device, mesh);

  return true;
}

bool ValidateOptions(const MeshLoaderOptions& options) {
  if (options.IndexBufferFormat != DXGI_FORMAT_R32_UINT && options.IndexBufferFormat != DXGI_FORMAT_R16_UINT) {
    return false;
  }

  return true;
}

bool LoadIndexBuffer32UInt(const aiMesh& imported_mesh, ID3D11Device* device, GpuMesh* mesh) {
  std::vector<uint32_t> indices;
  for (size_t face_index = 0; face_index < imported_mesh.mNumFaces; ++face_index) {
    for (size_t index_index = 0; index_index < imported_mesh.mFaces[face_index].mNumIndices; ++index_index) {
      indices.emplace_back(imported_mesh.mFaces[face_index].mIndices[index_index]);
    }
  }

  bool index_buffer_ok = CreateIndexBuffer(indices, device, mesh->IndexBuffer.GetAddressOf());
  if (!index_buffer_ok) {
    return false;
  }

  mesh->IndexBufferFormat = DXGI_FORMAT_R32_UINT;
  mesh->IndexCount = (uint32_t)indices.size();

  return true;
}

bool LoadIndexBuffer16UInt(const aiMesh& imported_mesh, ID3D11Device* device, GpuMesh* mesh) {
  std::vector<uint16_t> indices;
  for (size_t face_index = 0; face_index < imported_mesh.mNumFaces; ++face_index) {
    for (size_t index_index = 0; index_index < imported_mesh.mFaces[face_index].mNumIndices; ++index_index) {
      indices.emplace_back(static_cast<uint16_t>(imported_mesh.mFaces[face_index].mIndices[index_index]));
    }
  }

  bool index_buffer_ok = CreateIndexBuffer(indices, device, mesh->IndexBuffer.GetAddressOf());
  if (!index_buffer_ok) {
    return false;
  }

  mesh->IndexBufferFormat = DXGI_FORMAT_R16_UINT;
  mesh->IndexCount = (uint32_t)indices.size();

  return true;
}

bool LoadMesh(const filesystem::path& path, const MeshLoaderOptions& options, ID3D11Device* device, std::vector<GpuMesh>* meshes) {
  if (meshes == nullptr) {
    DXFW_TRACE(__FILE__, __LINE__, "Got a null mesh vector pointer", true);
    return false;
  }

  bool validate_ok = ValidateOptions(options);
  if (!validate_ok) {
    DXFW_TRACE(__FILE__, __LINE__, "Invalid mesh loading options", true);
    return false;
  }

  std::string path_string = path.string();
  
  AiLogStreamGuard log_stream_guard(aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr));

  std::unique_ptr<const aiScene, AiSceneDeleter> scene(aiImportFile(path_string.c_str(), aiProcessPreset_TargetRealtime_MaxQuality));

  if (!scene->HasMeshes()) {
    return false;
  }

  meshes->resize(scene->mNumMeshes);
  for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
    auto imported_mesh = scene->mMeshes[i];

    if (imported_mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE) {
      DXFW_TRACE(__FILE__, __LINE__, "Only triangular meshes are supported for loading", true);
      return false;
    }

    auto& mesh = meshes->back();

    uint32_t vertex_count = imported_mesh->mNumVertices;

    if (imported_mesh->HasPositions()) {
      bool prep_ok = PrepareFloat3VertexBuffer(imported_mesh->mVertices, vertex_count, VertexDataChannel::POSITIONS, device, &mesh);
      if (!prep_ok) {
        return false;
      }
    }

    if (imported_mesh->HasNormals()) {
      bool prep_ok = PrepareFloat3VertexBuffer(imported_mesh->mNormals, vertex_count, VertexDataChannel::NORMALS, device, &mesh);
      if (!prep_ok) {
        return false;
      }
    }

    if (imported_mesh->HasTangentsAndBitangents()) {
      bool tangent_prep_ok = PrepareFloat3VertexBuffer(imported_mesh->mTangents, vertex_count, VertexDataChannel::TANGENTS, device, &mesh);
      if (!tangent_prep_ok) {
        return false;
      }
      bool bitangent_prep_ok = PrepareFloat3VertexBuffer(imported_mesh->mBitangents, vertex_count, VertexDataChannel::BITANGENTS, device, &mesh);
      if (!bitangent_prep_ok) {
        return false;
      }
    }

    static_assert(MAX_TEXCOORDS <= AI_MAX_NUMBER_OF_TEXTURECOORDS, "MAX_TEXCOORDS must be no more than AI_MAX_NUMBER_OF_TEXTURECOORDS");
    for (size_t texture_index = 0; texture_index < MAX_TEXCOORDS; ++texture_index) {
      if (imported_mesh->mTextureCoords[texture_index] != nullptr) {
        if (imported_mesh->mNumUVComponents[texture_index] == 1) {
          PrepareFloat1VertexBuffer(imported_mesh->mTextureCoords[texture_index], vertex_count, GetTexCoordsChannel(texture_index), device, &mesh);
        } else if (imported_mesh->mNumUVComponents[texture_index] == 2) {
          PrepareFloat2VertexBuffer(imported_mesh->mTextureCoords[texture_index], vertex_count, GetTexCoordsChannel(texture_index), device, &mesh);
        } else {  // imported_mesh->mNumUVComponents[texture_index] == 3
          PrepareFloat3VertexBuffer(imported_mesh->mTextureCoords[texture_index], vertex_count, GetTexCoordsChannel(texture_index), device, &mesh);
        }
      }
    }

    static_assert(MAX_COLORS <= AI_MAX_NUMBER_OF_COLOR_SETS, "MAX_COLORS must be no more than AI_MAX_NUMBER_OF_COLOR_SETS");
    for (size_t color_index = 0; color_index < MAX_COLORS; ++color_index) {
      if (imported_mesh->mColors[color_index] != nullptr) {
        bool prep_ok = PrepareFloat4VertexBuffer(imported_mesh->mColors[color_index], vertex_count, GetColorsChannel(color_index), device, &mesh);
        if (!prep_ok) {
          return false;
        }
      }
    }

    mesh.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    if (options.IndexBufferFormat == DXGI_FORMAT_R32_UINT) {
      bool indices_ok = LoadIndexBuffer32UInt(*imported_mesh, device, &mesh);
      if (!indices_ok) {
        return false;
      }
    } else if (options.IndexBufferFormat == DXGI_FORMAT_R16_UINT) {
      bool indices_ok = LoadIndexBuffer16UInt(*imported_mesh, device, &mesh);
      if (!indices_ok) {
        return false;
      }
    } else {
      return false;
    }
  }

  return true;
}