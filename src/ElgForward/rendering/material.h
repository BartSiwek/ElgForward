#pragma once

#include "core/buffer.h"
#include "rendering/vertex_shader.h"
#include "rendering/pixel_shader.h"
#include "rendering/texture.h"

namespace Rendering {
namespace Material {

struct Material {
  Material() = default;
  ~Material() = default;

  Material(const Material&) = delete;
  Material& operator=(const Material&) = delete;

  Material(Material&&) = default;
  Material& operator=(Material&&) = default;

  VertexShader::Handle VertexShader = {};
  std::array<Texture::Handle, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> VertexShaderTextures = {};

  PixelShader::Handle PixelShader = {};
  std::array<Texture::Handle, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> PixelShaderTextures = {};

  Core::Buffer Data = {};
  size_t TypeHash = 0;
};

}  // namespace Material
}  // namespace Rendering
