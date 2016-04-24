#include "perspective_lens.h"

void PerspectiveLensUpdate(float zoom_factor, float aspect_ratio, float w, float h, float n, float f, float* effective_w, float* effective_h, DirectX::XMMATRIX* proj_matrix) {
  auto inv_zoom_factor = 1.0f / zoom_factor;
  if (aspect_ratio >= 1.0f) {
    *effective_h = inv_zoom_factor * h;
    *effective_w = aspect_ratio * h;
    *proj_matrix = DirectX::XMMatrixPerspectiveLH(*effective_w, *effective_h, n, f);
  } else {
    auto aspect_ratio_inv = 1.0f / aspect_ratio;
    *effective_w = inv_zoom_factor * w;
    *effective_h = aspect_ratio_inv * w;
    *proj_matrix = DirectX::XMMatrixPerspectiveLH(*effective_w, *effective_h, n, f);
  }
}
