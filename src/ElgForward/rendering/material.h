#pragma once

#include "rendering/vertex_shader.h"
#include "rendering/pixel_shader.h"
#include "rendering/constant_buffer.h"

namespace Rendering {

struct Material {
  size_t Hash;
  VertexShader::Handle VertexShader;
  PixelShader::Handle PixelShader;
  ConstantBuffer::Handle MaterialConstantBuffer;
};

}  // namespace Rendering
