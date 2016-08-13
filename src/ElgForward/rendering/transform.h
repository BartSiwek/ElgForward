#pragma once

#include "rendering/constant_buffer.h"

namespace Rendering {
namespace Transform {

struct Transform {
  ConstantBuffer::Handle TransformConstantBuffer;
};

}  // namespace Transform
}  // namespace Rendering
