#include "mesh_loader.h"

#include <fstream>
#include <memory>

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

bool LoadMesh(const filesystem::path& path, std::vector<Mesh>* meshes) {
  if (meshes == nullptr) {
    DXFW_TRACE(__FILE__, __LINE__, "Got a null mesh vector pointer", true);
    return false;
  }

  std::string path_string = path.string();
  
  AiLogStreamGuard log_stream_guard(aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr));

  std::unique_ptr<const aiScene, AiSceneDeleter> scene(aiImportFile(path_string.c_str(), aiProcessPreset_TargetRealtime_MaxQuality));

  if (!scene->HasMeshes()) {
    return false;
  }

  meshes->reserve(scene->mNumMeshes);
  for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
    auto imported_mesh = scene->mMeshes[i];

    if (imported_mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE) {
      return false;  // TODO: Handle other types
    }

    meshes->emplace_back(imported_mesh->mNumVertices, imported_mesh->mNumFaces * 3);
    auto& mesh = meshes->back();

    for (uint32_t j = 0; j < imported_mesh->mNumVertices; ++j) {
      mesh.Positions.emplace_back(imported_mesh->mVertices[j].x, imported_mesh->mVertices[j].y, imported_mesh->mVertices[j].z);
      mesh.Normals.emplace_back(imported_mesh->mNormals[j].x, imported_mesh->mNormals[j].y, imported_mesh->mNormals[j].z);
      mesh.TextureCoords.emplace_back(imported_mesh->mTextureCoords[0][j].x, imported_mesh->mTextureCoords[0][j].y);
    }

    for (uint32_t j = 0; j < imported_mesh->mNumFaces; ++j) {
      mesh.Indices.emplace_back(imported_mesh->mFaces[j].mIndices[0]);
      mesh.Indices.emplace_back(imported_mesh->mFaces[j].mIndices[1]);
      mesh.Indices.emplace_back(imported_mesh->mFaces[j].mIndices[2]);
    }
  }

  return true;
}