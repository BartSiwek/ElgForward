#pragma once

#include <DirectXMath.h>

namespace Rendering {
namespace Transform {

struct TransformAndInverseTranspose {
  DirectX::XMMATRIX Matrix;
  DirectX::XMMATRIX MatrixInverseTranspose;
};

}  // namespace Transform
}  // namespace Rendering
