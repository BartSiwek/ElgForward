#include "scene_loader.h"

#include <fstream>

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include <dxfw/dxfw.h>

#include "mesh.h"
#include "material.h"
#include "screen.h"

using json = nlohmann::json;

bool ReadJsonFile(const filesystem::path& path, json* json_scene) {
  std::ifstream input(path);

  if (!input.good()) {
    return false;
  }

  input >> *json_scene;

  return true;
}
bool ReadOptions(const json& json_options, MeshLoadOptions* options) {
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

void ReadMeshes(const json& json_scene, const filesystem::path& base_path, ID3D11Device* device, std::vector<MeshIdentifier>* mesh_identifiers) {
  auto& json_meshes = json_scene["meshes"];

  for (const auto& json_mesh : json_meshes) {
    bool is_valid_mesh_entry = json_mesh["prefix"].is_string()
                            && json_mesh["path"].is_string()
                            && json_mesh["options"].is_object();

    if (!is_valid_mesh_entry) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Invalid mesh entry %S", json_mesh.dump().c_str());
      continue;
    }

    auto& json_options = json_mesh["options"];

    MeshLoadOptions options;
    bool options_ok = ReadOptions(json_options, &options);
    if (!options_ok) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Invalid mesh options %S", json_options.dump().c_str());
      continue;
    }

    const std::string& prefix = json_mesh["prefix"];
    const std::string& path = json_mesh["path"];
      
    auto meshes_path = base_path / path;

    bool meshes_ok = CreateMeshes(prefix, meshes_path, options, device, mesh_identifiers);
    if (!meshes_ok) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error loading meshes from %S", meshes_path.string().c_str());
    }
  }
}

void ReadMaterials(const json& json_scene, const filesystem::path& base_path, ID3D11Device* device, std::vector<Material>* materials) {
  auto json_materials = json_scene["materials"];

  for (const auto& json_material : json_materials) {
    bool is_valid_material_entry = json_material["name"].is_string()
                                && json_material["vertex_shader"].is_string()
                                && json_material["pixel_shader"].is_string();

    if (!is_valid_material_entry) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Invalid material entry %S", json_material.dump().c_str());
      continue;
    }

    const std::string& name = json_material["name"];
    const std::string& vs_path = json_material["vertex_shader"];
    const std::string& ps_path = json_material["pixel_shader"];

    auto full_vs_path = base_path / vs_path;
    auto full_ps_path = base_path / ps_path;

    materials->emplace_back();
    auto& new_material = materials->back();
    bool material_ok = CreateMaterial(name, full_vs_path, full_ps_path, device, &new_material);
    if (!material_ok) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error loading material from [%S] and [%S]", full_vs_path.string().c_str(), full_ps_path.string().c_str());
    }
  }
}

void BuildDrawables(const json& json_scene, const std::vector<MeshIdentifier>& mesh_indetifiers, const std::vector<Material>& materials, ID3D11Device* device, std::vector<Drawable>* drawables) {
  auto json_drawables = json_scene["scene"];

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

    auto material_it = std::find_if(std::begin(materials), std::end(materials), [material_name_hash](const auto& material) {
      return material_name_hash == material.Hash;
    });

    drawables->emplace_back();
    auto& drawable = drawables->back();
    bool drawable_ok = CreateDrawable(mesh_indetifier_it->handle, *material_it, device, &drawable);
    if (!drawable_ok) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error creating drawable from mesh %S and material %S", mesh_name, material_name);
      continue;
    }
  }
}

void ReadCamera(const json& json_scene, DirectXState* state, TrackballCamera* camera, PerspectiveLens* lens) {
  const auto& json_camera = json_scene["camera"];
  
  const auto& json_lens = json_camera["prespective_lens"];
  if (json_lens.is_object()) {
    const auto& near_plane = json_lens["near_plane"];
    const auto& far_plane = json_lens["far_plane"];
    const auto& fov = json_lens["fov"];

    bool is_valid_lens_entry = near_plane.is_number_float()
                            && far_plane.is_number_float()
                            && fov.is_number_float();
    if (is_valid_lens_entry) {
      lens->SetFrustum(near_plane, far_plane, state->viewport.Width / state->viewport.Height, DirectX::XMConvertToRadians(fov));
    }
  }

  const auto& json_trackball_camera = json_camera["trackball_camera"];
  if (json_trackball_camera.is_object()) {
    const auto& radius = json_trackball_camera["radius"];
    if (radius.is_number_float()) {
      camera->SetRadius(radius);
    }
    
    const auto& json_position = json_trackball_camera["position"];
    if (json_position.is_array() && json_position.size() == 3) {
      const auto& x = json_position[0];
      const auto& y = json_position[1];
      const auto& z = json_position[2];
      if (x.is_number_float() && y.is_number_float() && z.is_number_float()) {
        camera->SetLocation(x, y, z);
      }
    } 
  }
}

void ConnectCameraToInput(DirectXState* state, TrackballCamera* camera, PerspectiveLens* lens) {
  Dxfw::RegisterMouseButtonCallback(state->window.get(), [camera, viewport = &state->viewport](dxfwWindow*, dxfwMouseButton button, dxfwMouseButtonAction action, int16_t x, int16_t y) {
    if (button == DXFW_RIGHT_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_DOWN) {
      camera->SetDesiredState(TrackballCameraOperation::Panning);
      camera->SetEndPoint(GetNormalizedScreenCoordinates(viewport->Width, viewport->Height, x, y));
    } else if (button == DXFW_RIGHT_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_UP) {
      camera->SetDesiredState(TrackballCameraOperation::None);
    } else if (button == DXFW_LEFT_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_DOWN) {
      camera->SetDesiredState(TrackballCameraOperation::Rotating);
      camera->SetEndPoint(GetNormalizedScreenCoordinates(viewport->Width, viewport->Height, x, y));
    } else if (button == DXFW_LEFT_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_UP) {
      camera->SetDesiredState(TrackballCameraOperation::None);
    } else if (button == DXFW_MIDDLE_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_DOWN) {
      camera->SetDesiredState(TrackballCameraOperation::Zooming);
      camera->SetEndPoint(GetNormalizedScreenCoordinates(viewport->Width, viewport->Height, x, y));
    } else if (button == DXFW_MIDDLE_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_UP) {
      camera->SetDesiredState(TrackballCameraOperation::None);
    }
  });

  Dxfw::RegisterMouseMoveCallback(state->window.get(), [camera, viewport = &state->viewport](dxfwWindow*, int16_t x, int16_t y) {
    camera->SetEndPoint(GetNormalizedScreenCoordinates(viewport->Width, viewport->Height, x, y));
  });

  Dxfw::RegisterMouseWheelCallback(state->window.get(), [lens](dxfwWindow*, int16_t, int16_t, int16_t delta) {
    if (delta > 0) {
      lens->SetZoomFactor(1.1f * lens->GetZoomFactor());
    } else {
      lens->SetZoomFactor(0.9f * lens->GetZoomFactor());
    }
  });
}

bool LoadScene(const filesystem::path& path, const filesystem::path& base_path, DirectXState* state, Scene* scene) {
  json json_scene;
  
  bool load_ok = ReadJsonFile(path, &json_scene);
  if (!load_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error reading scene file from %s", path.string());
    return false;
  }

  std::vector<MeshIdentifier> mesh_identifiers;
  ReadMeshes(json_scene, base_path, state->device.Get(), &mesh_identifiers);
  
  std::vector<Material> materials;
  ReadMaterials(json_scene, base_path, state->device.Get(), &materials);
  
  BuildDrawables(json_scene, mesh_identifiers, materials, state->device.Get(), &scene->drawables);
  
  ReadCamera(json_scene, state, &scene->camera, &scene->lens);

  ConnectCameraToInput(state, &scene->camera, &scene->lens);

  return true;
}