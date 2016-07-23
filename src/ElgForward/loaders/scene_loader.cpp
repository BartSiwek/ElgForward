#include "scene_loader.h"

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include <dxfw/dxfw.h>

#include "core/json_helpers.h"
#include "loaders/light_loader.h"
#include "loaders/camera_loader.h"
#include "loaders/material_loader.h"
#include "rendering/screen.h"
#include "rendering/material.h"
#include "mesh.h"

namespace Loaders {

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

    const auto& json_options = json_mesh["options"];

    MeshLoadOptions options;
    bool options_ok = ReadOptions(json_options, &options);
    if (!options_ok) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Invalid mesh options %S", json_options.dump().c_str());
      continue;
    }

    const std::string& prefix = json_mesh["prefix"];
    const std::string& path = json_mesh["path"];
      
    auto meshes_path = base_path / path;

    bool meshes_ok = CreateMeshes(prefix, meshes_path, options, state->device.Get(), mesh_identifiers);
    if (!meshes_ok) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error loading meshes from %S", meshes_path.string().c_str());
    }
  }
}

void ReadDrawableTransform(const nlohmann::json& json_transform, Rendering::Drawable* drawable) {
  // Translation
  DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();
  DirectX::XMMATRIX translation_inverse = DirectX::XMMatrixIdentity();

  const auto& json_translation = json_transform["translation"];

  float translation_values[3];
  if (Core::ReadFloat3(json_translation, translation_values)) {
    translation = DirectX::XMMatrixTranslation(translation_values[0], translation_values[1], translation_values[2]);
    translation_inverse = DirectX::XMMatrixTranslation(-translation_values[0], -translation_values[1], -translation_values[2]);
  } else {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid drawable translation %S", json_translation.dump().c_str());
  }

  // Rotation
  DirectX::XMMATRIX rotation = DirectX::XMMatrixIdentity();
  
  const auto& json_rotation = json_transform["rotation"];

  float rotation_values[4];
  if (Core::ReadFloat3(json_rotation, rotation_values)) {
    rotation = DirectX::XMMatrixRotationRollPitchYaw(rotation_values[0], rotation_values[1], rotation_values[2]);
  } else if (Core::ReadFloat4(json_rotation, rotation_values)) {
    auto axis = DirectX::XMVectorSet(rotation_values[0], rotation_values[1], rotation_values[2], 0.0f);
    rotation = DirectX::XMMatrixRotationAxis(axis, DirectX::XMConvertToRadians(rotation_values[3]));
  } else {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid drawable rotation %S", json_rotation.dump().c_str());
  }

  // Scaling
  DirectX::XMMATRIX scaling = DirectX::XMMatrixIdentity();
  DirectX::XMMATRIX scaling_inverse = DirectX::XMMatrixIdentity();
  
  const auto& json_scale = json_transform["scale"];

  float scale_values[3];
  if (Core::ReadFloat3(json_scale, scale_values)) {
    scaling = DirectX::XMMatrixScaling(scale_values[0], scale_values[1], scale_values[2]);
    scaling_inverse = DirectX::XMMatrixScaling(1.0f / scale_values[0], 1.0f / scale_values[1], 1.0f / scale_values[2]);
  } else {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid drawable scaling %S", json_scale.dump().c_str());
  }

  auto model_transform = scaling * rotation * translation;
  drawable->SetModelMatrix(model_transform);

  auto model_transform_inverse_transpose = scaling_inverse * rotation * DirectX::XMMatrixTranspose(translation_inverse);
  drawable->SetModelMatrixInverseTranspose(model_transform_inverse_transpose);
}

void BuildDrawables(const nlohmann::json& json_scene, const std::vector<MeshIdentifier>& mesh_indetifiers,
                    const std::vector<Rendering::Material>& materials, DirectXState* state,
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

    const std::string& mesh_name = json_drawable["mesh_name"];
    const std::string& material_name = json_drawable["material_name"];

    std::hash<std::string> hasher;
    auto mesh_name_hash = hasher(mesh_name);
    auto material_name_hash = hasher(material_name);

    auto mesh_indetifier_it = std::find_if(std::begin(mesh_indetifiers), std::end(mesh_indetifiers), [mesh_name_hash](const auto& identifier){
      return mesh_name_hash == identifier.Hash;
    });

    if (mesh_indetifier_it == std::end(mesh_indetifiers)) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error creating drawable from mesh %S and material %S - mesh not found", mesh_name.c_str(), material_name.c_str());
      continue;
    }

    auto material_it = std::find_if(std::begin(materials), std::end(materials), [material_name_hash](const auto& material) {
      return material_name_hash == material.Hash;
    });

    if (material_it == std::end(materials)) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error creating drawable from mesh %S and material %S - material not found", mesh_name.c_str(), material_name.c_str());
      continue;
    }

    drawables->emplace_back();
    auto& drawable = drawables->back();
    bool drawable_ok = CreateDrawable(mesh_indetifier_it->handle, *material_it, state->device.Get(), &drawable);
    if (!drawable_ok) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error creating drawable from mesh %S and material %S - CreateDrawable failed", mesh_name.c_str(), material_name.c_str());
      continue;
    }

    const auto& json_transform = json_drawable["transform"];
    if (json_transform.is_object()) {
      ReadDrawableTransform(json_transform, &drawable);
    }
  }
}

void ReadMaterials(const nlohmann::json& json_scene, const filesystem::path& base_path, DirectXState* state,
                   std::vector<Rendering::Material>* materials) {
  const auto& json_materials = json_scene["materials"];

  for (const auto& json_material : json_materials) {
    Rendering::Material new_material;
    bool material_ok = ReadMaterial(json_material, base_path, state, &new_material);
    if (!material_ok) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error loading material [%S]", json_material.dump().c_str());
    }

    materials->emplace_back(new_material);
  }
}

void ReadLights(const nlohmann::json& json_scene, const filesystem::path& base_path, Scene* scene) {
  const auto& json_lights = json_scene["lights"];

  if (!json_lights.is_string()) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid lights entry [%S]", json_lights.dump().c_str());
    return;
  }

  const std::string& lights_relative_path = json_lights;
  auto lights_path = base_path / lights_relative_path;
  bool lights_ok = ReadLightsFromFile(lights_path, scene->DirectionalLightsStructuredBuffer,
                                      scene->SpotLightsStructuredBuffer, scene->PointLightsStructuredBuffer);
  if (!lights_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error reading lights from [%S]", lights_path.c_str());
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
  
  std::vector<Rendering::Material> materials;
  ReadMaterials(json_scene, base_path, state, &materials);
  
  ReadLights(json_scene, base_path, scene);

  BuildDrawables(json_scene, mesh_identifiers, materials, state, &scene->Drawables);
  
  ReadCamera(json_scene, base_path, state, scene);

  return true;
}

}  // namespace Loaders
