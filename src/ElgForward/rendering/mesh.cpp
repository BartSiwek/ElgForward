#include "mesh.h"

#include "core/hash.h"
#include "core/resource_array.h"
#include "core/handle_cache.h"

namespace Rendering {
namespace Mesh {

Core::ResourceArray<Handle, std::unique_ptr<MeshData>, 255> g_storage_;
Core::HandleCache<size_t, Handle> g_cache_;

Handle Create(size_t mesh_hash, std::unique_ptr<MeshData>&& data) {
  auto cached_handle = g_cache_.Get(mesh_hash);
  if (cached_handle.IsValid()) {
    return cached_handle;
  }

  auto new_mesh_handle = g_storage_.Add(std::move(data));
  g_cache_.Set(mesh_hash, new_mesh_handle);
  return new_mesh_handle;
}

Handle Exists(size_t mesh_hash) {
  return g_cache_.Get(mesh_hash);
}

MeshData* Retreive(Handle handle) {
  return g_storage_.Get(handle).get();
}

}  // namespace Mesh
}  // namespace Rendering
