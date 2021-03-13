#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include <nlohmann/json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "rendering/texture.h"

namespace Loaders {

struct TextureIdentifier {
  size_t Hash;
  Rendering::Texture::Handle Texture;
};

bool ReadTexturesFromFile(const filesystem::path& textures_path, const filesystem::path& base_path, ID3D11Device* device, std::vector<TextureIdentifier>* textures);

bool ReadTexturesFromJson(const nlohmann::json& json_textures, const filesystem::path& base_path, ID3D11Device* device, std::vector<TextureIdentifier>* textures);

}  // namespace Loaders
