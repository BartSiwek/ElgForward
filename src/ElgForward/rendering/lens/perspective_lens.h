#pragma once

#include <cmath>

#include <d3d11.h>

#include <DirectXMath.h>

namespace Rendering {
namespace Lens {

void PerspectiveLensUpdate(float zoom_factor, float aspect_ratio, float w, float h, float n, float f, float* frustum_width, float* frustum_height, DirectX::XMMATRIX* proj_matrix);

class PerspectiveLens {
public:
  PerspectiveLens()
      : m_proj_matrix_(DirectX::XMMatrixIdentity()),
        m_zoom_factor_(1.0f),
        m_near_(1.0f),
        m_far_(2.0f),
        m_frustum_width_(1.0f),
        m_frustum_height_(1.0f) {
  }

  ~PerspectiveLens() = default;

  PerspectiveLens(const PerspectiveLens&) = delete;
  PerspectiveLens& operator=(const PerspectiveLens&) = delete;

  PerspectiveLens(PerspectiveLens&&) = default;
  PerspectiveLens& operator=(PerspectiveLens&&) = default;

  void SetFrustum(float near_plane, float far_plane, float aspect_ratio, float horizontal_fov) {
    m_near_ = near_plane;
    m_far_ = far_plane;

    auto horizontal_fol_half_tan = std::tan(horizontal_fov / 2.0f);
    m_frustum_width_ = 2.0f * m_near_ * horizontal_fol_half_tan;
    m_frustum_height_ = 2.0f * m_near_ * horizontal_fol_half_tan / aspect_ratio;
  }

  void SetZoomFactor(float zoom_factor) {
    m_zoom_factor_ = zoom_factor;
  }

  float GetZoomFactor() const {
    return m_zoom_factor_;
  }

  void UpdateMatrices(float aspect_ratio) {
    float frustum_width;
    float frustum_height;
    PerspectiveLensUpdate(m_zoom_factor_, aspect_ratio, m_frustum_width_, m_frustum_height_, m_near_, m_far_, &frustum_width, &frustum_height, &m_proj_matrix_);
  }

  void UpdateMatrices(float aspect_ratio, float* frustum_width, float* frustum_height) {
    PerspectiveLensUpdate(m_zoom_factor_, aspect_ratio, m_frustum_width_, m_frustum_height_, m_near_, m_far_, frustum_width, frustum_height, &m_proj_matrix_);
  }

  const DirectX::XMMATRIX& GetProjectionMatrix() const {
    return m_proj_matrix_;
  }

private:
  // Finished product
  DirectX::XMMATRIX m_proj_matrix_;

  // Zoom
  float m_zoom_factor_;

  // Projection
  float m_near_;
  float m_far_;
  float m_frustum_width_;
  float m_frustum_height_;
};

}  // namespace Lens
}  // namespace Rendering
