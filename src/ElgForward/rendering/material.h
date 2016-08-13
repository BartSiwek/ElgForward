#pragma once

#include "rendering/vertex_shader.h"
#include "rendering/pixel_shader.h"
#include "rendering/constant_buffer.h"

namespace Rendering {
namespace Material {

struct Material {
  VertexShader::Handle VertexShader;
  PixelShader::Handle PixelShader;
  ConstantBuffer::Handle MaterialConstantBuffer;
};

}  // namespace Material
}  // namespace Rendering
