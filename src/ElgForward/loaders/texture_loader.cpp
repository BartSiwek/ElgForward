#include "texture_loader.h"

#include <unordered_map>

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4244)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)

#include <dxfw/dxfw.h>

#include "core/json_helpers.h"
#include "core/filesystem.h"
#include "rendering/texture.h"

using namespace Rendering;

namespace Loaders {

struct TextureDeleter {
  void operator()(unsigned char* ptr) {
    stbi_image_free(ptr);
  }
};

DXGI_FORMAT StbiComponentsToDxgiFormat(int image_components) {
  switch (image_components) {
    case 1:
      return DXGI_FORMAT_R8_UNORM;
    case 2:
      return DXGI_FORMAT_R8G8_UNORM;
    case 3:
      return DXGI_FORMAT_UNKNOWN;
    case 4:
      return DXGI_FORMAT_R8G8B8A8_UNORM;
    default:
      return DXGI_FORMAT_UNKNOWN;
  }
}

bool ReadWithStbi(const std::string& path, const filesystem::path& base_path,
                  std::vector<std::unique_ptr<unsigned char, TextureDeleter>>* textures,
                  std::vector<Rendering::Texture::ImageData>* data) {
  auto full_path = base_path / path;

  int current_image_width;
  int current_image_height;
  int current_image_components;
  auto image = stbi_load(full_path.generic_string().c_str(), &current_image_width, &current_image_height, &current_image_components, STBI_default);
  textures->emplace_back(image);

  auto dxgi_format = StbiComponentsToDxgiFormat(current_image_components);
  if (dxgi_format == DXGI_FORMAT_UNKNOWN) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Unsupported number of components from STBI: %d", current_image_components);
    return false;
  }

  data->emplace_back(current_image_width, current_image_height, current_image_components, dxgi_format, image);

  return true;
}

Rendering::Texture::Handle ReadSingleTexture(size_t name_hash, const std::string& path, const filesystem::path& base_path, ID3D11Device* device) {
  std::vector<std::unique_ptr<unsigned char, TextureDeleter>> textures;
  std::vector<Rendering::Texture::ImageData> data = {};

  auto read_ok = ReadWithStbi(path, base_path, &textures, &data);
  if (!read_ok) {
    return {};
  }

  return Rendering::Texture::Create(name_hash, data, device);
}

Rendering::Texture::Handle ReadMipmapTexture(size_t name_hash, const nlohmann::json::array_t& path_array, const filesystem::path& base_path, ID3D11Device* device) {
  std::vector<std::unique_ptr<unsigned char, TextureDeleter>> textures;
  std::vector<Rendering::Texture::ImageData> data = {};

  for (const auto& path : path_array) {
    auto read_ok = ReadWithStbi(path, base_path, &textures, &data);
    if (!read_ok) {
      return {};
    }
  }

  return Rendering::Texture::Create(name_hash, data, device);
}

bool ReadTexturesFromJson(const nlohmann::json& json_textures, const filesystem::path& base_path, ID3D11Device* device, std::vector<TextureIdentifier>* textures) {
  const auto& json_textures_array = json_textures.value("textures", nlohmann::json::array({}));

  if (!json_textures_array.is_array()) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid textures JSON %S", json_textures_array.dump().c_str());
    return false;
  }

  for (const auto& json_texture : json_textures_array) {
    if (!json_texture.is_object()) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Invalid textures entry %S", json_texture.dump().c_str());
      continue;
    }

    const auto& json_texture_name_it = json_texture.find("name");
    if (json_texture_name_it == json_texture.end() || !json_texture_name_it->is_string()) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Invalid texture name %S", json_texture_name_it->dump().c_str());
      continue;
    }

    const auto& json_texture_path_it = json_texture.find("filename");
    if (json_texture_path_it == json_texture.end() || !(json_texture_path_it->is_string() || json_texture_path_it->is_array())) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Invalid texture filename(s) %S", json_texture_path_it->dump().c_str());
      continue;
    }

    TextureIdentifier identifier;
    identifier.Hash = std::hash<std::string>()(*json_texture_name_it);

    if (json_texture_path_it->is_string()) {
      identifier.Texture = ReadSingleTexture(identifier.Hash, *json_texture_path_it, base_path, device);
    } else {  // Array
      identifier.Texture = ReadMipmapTexture(identifier.Hash, *json_texture_path_it, base_path, device);
    }

    if (!identifier.Texture.IsValid()) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error creating texture %S", json_texture_name_it->dump().c_str());
      continue;
    }

    textures->emplace_back(std::move(identifier));
  }

  return true;
}

bool ReadTexturesFromFile(const filesystem::path& lights_path, const filesystem::path& base_path, ID3D11Device* device, std::vector<TextureIdentifier>* textures) {
  nlohmann::json json_textures;

  bool load_ok = Core::ReadJsonFile(lights_path, &json_textures);
  if (!load_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error reading textures file from %s", lights_path.string());
    return false;
  }

  return ReadTexturesFromJson(json_textures, base_path, device, textures);
}

}  // namespace Loaders
