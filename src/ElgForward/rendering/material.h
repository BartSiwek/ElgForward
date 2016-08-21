#pragma once

#include "core/buffer.h"
#include "rendering/vertex_shader.h"
#include "rendering/pixel_shader.h"

namespace Rendering {
namespace Material {

struct Material {
  VertexShader::Handle VertexShader;
  PixelShader::Handle PixelShader;
  Core::Buffer Data;
  size_t TypeHash;
};

}  // namespace Material
}  // namespace Rendering
