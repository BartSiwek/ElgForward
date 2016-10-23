#include "perspective_lens.h"

namespace Rendering {
namespace Lens {

void PerspectiveLensUpdate(float zoom_factor, float aspect_ratio, float w, float h, float n, float f, float* frustum_width, float* frustum_height, DirectX::XMMATRIX* proj_matrix) {
  auto inv_zoom_factor = 1.0f / zoom_factor;
  if (aspect_ratio >= 1.0f) {
    *frustum_height = inv_zoom_factor * h;
    *frustum_width = aspect_ratio * *frustum_height;
    *proj_matrix = DirectX::XMMatrixPerspectiveLH(*frustum_width, *frustum_height, n, f);
  } else {
    auto aspect_ratio_inv = 1.0f / aspect_ratio;
    *frustum_width = inv_zoom_factor * w;
    *frustum_height = aspect_ratio_inv * *frustum_width;
    *proj_matrix = DirectX::XMMatrixPerspectiveLH(*frustum_width, *frustum_height, n, f);
  }
}

}  // namespace Lens
}  // namespace Rendering
