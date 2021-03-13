#include "scene_loader.h"

#pragma warning(push)
#pragma warning(disable: 4706)
#include <nlohmann/json.hpp>
#pragma warning(pop)

#include <dxfw/dxfw.h>

#include "core/json_helpers.h"
#include "loaders/light_loader.h"
#include "loaders/camera_loader.h"
#include "loaders/material_loader.h"
#include "loaders/mesh_loader.h"
#include "loaders/transform_loader.h"
#include "loaders/texture_loader.h"
#include "rendering/screen.h"
#include "rendering/material.h"
#include "rendering/mesh.h"
#include "rendering/transform.h"
#include "rendering/transform_and_inverse_transpose.h"

namespace Loaders {

void ReadMeshes(const nlohmann::json& json_scene, const filesystem::path& base_path, DirectXState* state, std::vector<MeshIdentifier>* mesh_identifiers) {
  const auto& json_meshes = json_scene["meshes"];

  for (const auto& json_mesh : json_meshes) {
    bool is_valid_mesh_entry = json_mesh["prefix"].is_string()
                            && json_mesh["path"].is_string()
                            && json_mesh["options"].is_object();

    if (!is_valid_mesh_entry) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Invalid mesh entry %S", json_mesh.dump().c_str());
      continue;
    }

    bool mesh_ok = ReadMesh(json_mesh, base_path, state->device.Get(), mesh_identifiers);
    if (!mesh_ok) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error reading mesh from %S", json_mesh.dump().c_str());
      continue;
    }
  }
}

void ReadDrawableTransform(const std::string& drawable_name, const nlohmann::json& json_drawable, DirectXState* state, Rendering::Transform::Transform* transform) {
  const auto& json_transform = json_drawable["transform"];
  
  if (!json_transform.is_object()) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid drawable transform entry [%S]", json_transform.dump().c_str());
    return;
  }

  bool transform_ok = ReadTransform(drawable_name, json_transform, state->device.Get(), transform);

  if (!transform_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error reading drawable transform entry [%S]", json_transform.dump().c_str());
  }
}

void BuildDrawables(const nlohmann::json& json_scene, const std::vector<MeshIdentifier>& mesh_indetifiers,
                    const std::vector<MaterialIdentifier>& materials, DirectXState* state,
                    std::vector<Rendering::Drawable>* drawables) {
  const auto& json_drawables = json_scene["scene"];

  for (const auto& json_drawable : json_drawables) {
    bool is_valid_drawable_entry = json_drawable["name"].is_string()
                                && json_drawable["mesh_name"].is_string()
                                && json_drawable["material_name"].is_string();

    if (!is_valid_drawable_entry) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Invalid drawable entry %S", json_drawable.dump().c_str());
      continue;
    }

    const std::string& drawable_name = json_drawable["name"];
    const std::string& mesh_name = json_drawable["mesh_name"];
    const std::string& material_name = json_drawable["material_name"];

    std::hash<std::string> hasher;
    auto drawable_name_hash = hasher(drawable_name);
    auto mesh_name_hash = hasher(mesh_name);
    auto material_name_hash = hasher(material_name);

    auto mesh_indetifier_it = std::find_if(std::begin(mesh_indetifiers), std::end(mesh_indetifiers), [mesh_name_hash](const auto& identifier){
      return mesh_name_hash == identifier.Hash;
    });

    if (mesh_indetifier_it == std::end(mesh_indetifiers)) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error creating drawable from mesh %S and material %S - mesh not found", mesh_name.c_str(), material_name.c_str());
      continue;
    }

    auto mesh = Rendering::Mesh::Retreive(mesh_indetifier_it->handle);

    auto material_identifier_it = std::find_if(std::begin(materials), std::end(materials), [material_name_hash](const auto& material) {
      return material_name_hash == material.Hash;
    });

    if (material_identifier_it == std::end(materials)) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error creating drawable from mesh %S and material %S - material not found", mesh_name.c_str(), material_name.c_str());
      continue;
    }

    Rendering::Transform::Transform transform;
    ReadDrawableTransform(drawable_name, json_drawable, state, &transform);

    Rendering::Drawable drawable;
    bool drawable_ok = CreateDrawable(drawable_name_hash, *mesh, material_identifier_it->Hash, material_identifier_it->Material, transform, state->device.Get(), &drawable);
    if (!drawable_ok) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error creating drawable from mesh %S and material %S - CreateDrawable failed", mesh_name.c_str(), material_name.c_str());
      continue;
    }

    drawables->emplace_back(std::move(drawable));
  }
}

void ReadMaterials(const nlohmann::json& json_scene, const filesystem::path& base_path,
                   const std::vector<TextureIdentifier>& textures, DirectXState* state,
                   std::vector<MaterialIdentifier>* materials) {
  const auto& json_materials = json_scene["materials"];

  for (const auto& json_material : json_materials) {
    MaterialIdentifier new_material;
    bool material_ok = ReadMaterial(json_material, base_path, textures, state->device.Get(), &new_material);
    if (!material_ok) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error loading material [%S]", json_material.dump().c_str());
    }

    materials->emplace_back(std::move(new_material));
  }
}

void ReadLights(const nlohmann::json& json_scene, const filesystem::path& base_path, Scene* scene) {
  auto lights_it = json_scene.find("lights");
  if (lights_it == json_scene.end()) {
    return;
  }

  if (!lights_it->is_string()) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid lights entry [%S]", lights_it->dump().c_str());
    return;
  }

  const std::string& lights_relative_path = *lights_it;
  auto lights_path = base_path / lights_relative_path;
  bool lights_ok = ReadLightsFromFile(lights_path, scene->DirectionalLightsStructuredBuffer,
                                      scene->SpotLightsStructuredBuffer, scene->PointLightsStructuredBuffer);
  if (!lights_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error reading lights from [%S]", lights_path.c_str());
  }
}

void ReadTextures(const nlohmann::json& json_scene, const filesystem::path& base_path, DirectXState* state, std::vector<TextureIdentifier>* textures) {
  auto textures_it = json_scene.find("textures");
  if (textures_it == json_scene.end()) {
    return;
  }

  if (!textures_it->is_string()) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid textures entry [%S]", textures_it->dump().c_str());
    return;
  }

  const std::string& textures_relative_path = *textures_it;
  auto textures_path = base_path / textures_relative_path;
  bool textures_ok = ReadTexturesFromFile(textures_path, base_path, state->device.Get(), textures);

  if (!textures_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error reading textures from [%S]", textures_path.c_str());
  }
}

void ReadCamera(const nlohmann::json& json_scene, const filesystem::path& base_path, DirectXState* state, Scene* scene) {
  const auto& json_camera = json_scene["camera"];

  if (!json_camera.is_object()) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid camera entry [%S]", json_camera.dump().c_str());
    return;
  }

  bool camera_ok = ReadCamera(json_camera, base_path, state, &scene->Camera, &scene->Lens, &scene->CameraScript);
  if (!camera_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error reading camera from [%S]", json_camera.dump().c_str());
  }
}

bool LoadScene(const filesystem::path& path, const filesystem::path& base_path, DirectXState* state, Scene* scene) {
  nlohmann::json json_scene;
  
  bool load_ok = Core::ReadJsonFile(path, &json_scene);
  if (!load_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error reading scene file from %S", path.c_str());
    return false;
  }

  std::vector<MeshIdentifier> mesh_identifiers;
  ReadMeshes(json_scene, base_path, state, &mesh_identifiers);
  
  std::vector<TextureIdentifier> textures;
  ReadTextures(json_scene, base_path, state, &textures);

  std::vector<MaterialIdentifier> materials;
  ReadMaterials(json_scene, base_path, textures, state, &materials);
  
  ReadLights(json_scene, base_path, scene);

  BuildDrawables(json_scene, mesh_identifiers, materials, state, &scene->Drawables);
  
  ReadCamera(json_scene, base_path, state, scene);

  return true;
}

}  // namespace Loaders
