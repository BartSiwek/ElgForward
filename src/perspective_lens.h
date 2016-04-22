#pragma once

#include <cmath>

#include <d3d11.h>

#include <DirectXMath.h>

class PerspectiveLens {
public:
  PerspectiveLens()
      : m_width_(1),
        m_height_(1),
        m_near_(1.0f),
        m_far_(2.0f),
        m_zoom_factor_(1.0f),
        m_tg_half_horizontal_fov_(1.0f),
        m_tg_half_vertical_fov_(1.0f),
        m_proj_matrix_(DirectX::XMMatrixIdentity()),
        m_viewport_() {
  }

  ~PerspectiveLens() = default;

  PerspectiveLens(const PerspectiveLens&) = delete;
  PerspectiveLens& operator=(const PerspectiveLens&) = delete;

  PerspectiveLens(PerspectiveLens&&) = default;
  PerspectiveLens& operator=(PerspectiveLens&&) = default;

  void SetViewportSize(uint32_t width, uint32_t height) {
    m_width_ = width;
    m_height_ = height;
  }

  void SetFrustum(float near_plane, float far_plane, float aspect_ratio, float horizontal_fov) {
    m_near_ = near_plane;
    m_far_ = far_plane;
    m_tg_half_horizontal_fov_ = std::tan(horizontal_fov / 2.0f);
    m_tg_half_vertical_fov_ = m_tg_half_horizontal_fov_ / aspect_ratio;
  }

  void SetZoomFactor(float zoom_factor) {
    m_zoom_factor_ = zoom_factor;
  }

  float GetZoomFactor() const {
    return m_zoom_factor_;
  }

  float GetFrustumWidth() const {
    return m_frustum_width_;
  }

  float GetFrustumHeight() const {
    return m_frustum_height_;
  }

  void UpdateMatricesAndViewport() {
    UpdateProjMatrix();
    UpdateViewport();
  }

  const DirectX::XMMATRIX& GetProjectionMatrix() const {
    return m_proj_matrix_;
  }

  const D3D11_VIEWPORT& GetViewport() const {
    return m_viewport_;
  }

private:
  void UpdateProjMatrix() {
    float inv_zoom_factor = 1.0f / m_zoom_factor_;
    if (m_width_ >= m_height_) {
      auto aspect_ratio = static_cast<float>(m_width_) / static_cast<float>(m_height_);
      m_frustum_height_ = inv_zoom_factor * 2.0f * m_near_ * m_tg_half_vertical_fov_;
      m_frustum_width_ = aspect_ratio * m_frustum_height_;
      m_proj_matrix_ = DirectX::XMMatrixPerspectiveLH(m_frustum_width_, m_frustum_height_, m_near_, m_far_);
    } else {
      auto aspect_ratio_inv = static_cast<float>(m_height_) / static_cast<float>(m_width_);
      m_frustum_width_ = inv_zoom_factor * 2.0f * m_near_ * m_tg_half_horizontal_fov_;
      m_frustum_height_ = aspect_ratio_inv * m_frustum_width_;
      m_proj_matrix_ = DirectX::XMMatrixPerspectiveLH(m_frustum_width_, m_frustum_height_, m_near_, m_far_);
    }
  }

  void UpdateViewport() {
    ZeroMemory(&m_viewport_, sizeof(D3D11_VIEWPORT));
    m_viewport_.TopLeftX = 0;
    m_viewport_.TopLeftY = 0;
    m_viewport_.Width = static_cast<float>(m_width_);
    m_viewport_.Height = static_cast<float>(m_height_);
  }

  // Finished product
  DirectX::XMMATRIX m_proj_matrix_;
  D3D11_VIEWPORT m_viewport_;

  // Frustum
  float m_frustum_width_;
  float m_frustum_height_;

  // Projection
  float m_near_;
  float m_far_;
  float m_zoom_factor_;
  float m_tg_half_horizontal_fov_;
  float m_tg_half_vertical_fov_;

  // Viewport
  uint32_t m_width_;
  uint32_t m_height_;
};