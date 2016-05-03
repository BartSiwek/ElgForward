#include "perspective_lens.h"

void PerspectiveLensUpdate(float zoom_factor, float aspect_ratio, float w, float h, float n, float f, DirectX::XMFLOAT2* frustum_size, DirectX::XMMATRIX* proj_matrix) {
  auto inv_zoom_factor = 1.0f / zoom_factor;
  if (aspect_ratio >= 1.0f) {
    frustum_size->y = inv_zoom_factor * h;
    frustum_size->x = aspect_ratio * frustum_size->y;
    *proj_matrix = DirectX::XMMatrixPerspectiveLH(frustum_size->x, frustum_size->y, n, f);
  } else {
    auto aspect_ratio_inv = 1.0f / aspect_ratio;
    frustum_size->x = inv_zoom_factor * w;
    frustum_size->y = aspect_ratio_inv * frustum_size->x;
    *proj_matrix = DirectX::XMMatrixPerspectiveLH(frustum_size->x, frustum_size->y, n, f);
  }
}
