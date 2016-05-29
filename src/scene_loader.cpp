#include "scene_loader.h"

#include <fstream>

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include <dxfw/dxfw.h>

#include "gpu_mesh.h"

using json = nlohmann::json;

bool ReadJsonFile(const filesystem::path& path, json* json_scene) {
  std::ifstream input(path);

  if (!input.good()) {
    return false;
  }

  input >> *json_scene;

  return true;
}

bool ReadMeshes(const json& json_scene) {
  auto json_meshes = json_scene["meshes"];

  for (const auto& json_mesh : json_meshes) {
    if (json_mesh["name"].is_string() && json_mesh["filename"].is_string()) {
      const std::string& name = json_mesh["name"];
      const std::string& filename = json_mesh["filename"];

      

      DXFW_TRACE(__FILE__, __LINE__, false, "Found mesh container %S with filename %S", name.c_str(), filename.c_str());
    }
  }

  return true;
}

bool ReadMaterials(const json& json_scene) {
  auto json_materials = json_scene["materials"];

  DXFW_TRACE(__FILE__, __LINE__, false, "Found %d materials", json_materials.size());

  return true;
}

bool BuildDrawables(const json& json_scene) {
  auto json_drawables = json_scene["scene"];

  DXFW_TRACE(__FILE__, __LINE__, false, "Found %d drawables", json_drawables.size());

  return true;
}

bool LoadScene(const filesystem::path& path, Scene* /* scene */) {
  json json_scene;
  
  bool load_ok = ReadJsonFile(path, &json_scene);
  if (!load_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error reading scene file from %s", path.string());
    return false;
  }

  bool meshes_ok = ReadMeshes(json_scene);
  if (!meshes_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error reading meshes for scene");
    return false;
  }

  bool materials_ok = ReadMaterials(json_scene);
  if (!materials_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error reading materials for scene");
    return false;
  }

  bool drawables_ok = BuildDrawables(json_scene);
  if (!drawables_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error creating drawables for scene");
    return false;
  }

  return true;
}