#pragma once

/*
MODES:
- Pan
- Rotate
- Zoom

EVENTS:
- On client size changed - register new size
- On mouse down - call PanStart / RotationStart / ZoomStart
- On mouse move - call PositionUpdate
- On mouse up - call PanEnd / RotationEnd / ZoomEnd

Functions:
- Get model matrix
- Get projection matrix

Possible members:
- Mode = enum { PAN, ROTATE, ZOOM }
- Current view matrix
- Current projection matrix
- Start point (of any operation)
- Current point (of any operation)
- Additional view matrix (to handle pan and rotate between Start and End)
- Additional projection matrix (to handle zoom between Start and End)

Rotation:
- Remember the start point on unit sphere 
- On move compute the new point on unit sphere
- Calculate the rotation axis and angle
  - Axis = cross product of the two points - its unit since the points are on unit sphere
  - Angle - take dot product
- Convert axis and angle to quaternion
- Update the current rotation quaternion with this info

Pan:
- Keep the current center point
- With mouse down remember the current start point in screen coords
- With mouse move:
  - Take the new pos in screen coord
  - Subrstract it from start pos to get the vector
  - Translate vector to a [0,1]x[0,1] space
  - Multiply this by width & height of the near plane to get the vector in world coords

Zoom:
- Keep the zoom factor - default = 1.0
- Map the difference between starting and current point on the screen to [-1, 1] according to the vertical axis
- Map [-1, 1] to some reasonable range [0.5, 2] for example
- 

*/

#include <cmath>

#include <d3d11.h>

#include <DirectXMath.h>

class TrackballCamera {
public:
  TrackballCamera()
      : m_current_state_(State::None),
        m_start_point_(0.0f, 0.0f),
        m_end_point_(0.0f, 0.0f),
        m_width_(1),
        m_height_(1),
        m_center_(0.0f, 0.0f, 0.0f),
        m_rotation_quaterion_(0.0f, 0.0f, 0.0f, 1.0f),  // Quaternion identity
        m_zoom_factor_(1.0f),
        m_near_(1.0f),
        m_far_(2.0f),
        m_tg_half_horizontal_fov_(1.0f),
        m_tg_half_vertical_fov_(1.0f),
        m_view_matrix_(DirectX::XMMatrixIdentity()),
        m_inverse_view_matrix_(DirectX::XMMatrixIdentity()) {
  }

  ~TrackballCamera() = default;

  TrackballCamera(const TrackballCamera&) = delete;
  TrackballCamera& operator=(const TrackballCamera&) = delete;

  TrackballCamera(TrackballCamera&&) = default;
  TrackballCamera& operator=(TrackballCamera&&) = default;

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

  void SetLocation(float x, float y, float z) {
    m_center_.x = x;
    m_center_.y = y;
    m_center_.z = z;
  }

  void LookAt(float x, float y, float z) {
    auto zAxis = DirectX::XMVectorSet(0, 0, 1, 0);
    auto c = DirectX::XMLoadFloat3(&m_center_);
    auto v = DirectX::XMPlaneNormalize(DirectX::XMVectorSubtract(DirectX::XMVectorSet(x, y, z, 0), c));

    auto axis = DirectX::XMVector3Cross(zAxis, v);
    auto angle = DirectX::XMScalarACos(DirectX::XMVectorGetX(DirectX::XMVector3Dot(zAxis, v)));

    DirectX::XMStoreFloat4(&m_rotation_quaterion_, DirectX::XMQuaternionRotationAxis(axis, angle));
  }

  void StartPan(uint32_t x, uint32_t y) {
    StartOperation(State::Panning, x, y);
  }

  void EndPan() {
    if (m_current_state_ == State::Panning) {
      auto frustum_size = GetFrustumSize(m_zoom_factor_);
      auto t = GetCurrentTranslationVector(frustum_size);

      auto center = DirectX::XMLoadFloat3(&m_center_);
      auto new_center = DirectX::XMVectorSubtract(center, t);

      DirectX::XMStoreFloat3(&m_center_, new_center);

      EndOperation();
    }
  }

  void StartRotation(uint32_t x, uint32_t y) {
    StartOperation(State::Rotating, x, y);
  }

  void EndRotation() {
    if (m_current_state_ == State::Rotating) {
      auto q = DirectX::XMLoadFloat4(&m_rotation_quaterion_);
      q = DirectX::XMQuaternionMultiply(q, GetRotationQuaternion());
      DirectX::XMStoreFloat4(&m_rotation_quaterion_, q);
      EndOperation();
    }
  }

  void StartZoom(uint32_t x, uint32_t y) {
    StartOperation(State::Zooming, x, y);
  }

  void EndZoom() {
    if (m_current_state_ == State::Zooming) {
      m_zoom_factor_ = GetZoomFactor();
      EndOperation();
    }
  }

  void UpdatePosition(uint32_t x, uint32_t y) {
    if (m_current_state_ != State::None) {
      m_end_point_ = GetNormalizedScreenCoordinates(x, y);
    }
  }

  void UpdateMatricesAndViewport() {
    auto zoom_factor = GetZoomFactor();
    auto frustum_size = GetFrustumSize(zoom_factor);
    auto center = DirectX::XMLoadFloat3(&m_center_);
    auto q = DirectX::XMLoadFloat4(&m_rotation_quaterion_);

    UpdateViewMatrix(center, q, frustum_size);
  }

  const DirectX::XMMATRIX& GetViewMatrix() const {
    return m_view_matrix_;
  }

  const DirectX::XMMATRIX& GetViewMatrixInverse() const {
    return m_inverse_view_matrix_;
  }

private:
  enum class State {
    None,
    Panning,
    Rotating,
    Zooming,
  };
  
  void StartOperation(State operation, uint32_t x, uint32_t y) {
    if (m_current_state_ == State::None) {
      m_current_state_ = operation;
      m_start_point_ = GetNormalizedScreenCoordinates(x, y);
      m_end_point_ = m_start_point_;
    }
  }

  void EndOperation() {
    if (m_current_state_ != State::None) {
      m_current_state_ = State::None;
      m_start_point_.x = m_start_point_.y = 0.0f;
      m_end_point_.x = m_end_point_.y = 0.0f;
    }
  }

  DirectX::XMFLOAT2 GetNormalizedScreenCoordinates(uint32_t x, uint32_t y) {
    return DirectX::XMFLOAT2(static_cast<float>(2 * x) / static_cast<float>(m_width_) - 1.0f,
                             1.0f - static_cast<float>(2 * y) / static_cast<float>(m_height_));
    
  }

  float GetZoomFactor() {
    if (m_current_state_ != State::Zooming) {
      return m_zoom_factor_;
    }

    auto d = m_end_point_.y - m_start_point_.y;
    auto extra_factor = d / 16.0f + 3.0f * d / 8.0f + 1.0f;

    return extra_factor * m_zoom_factor_;
  }

  DirectX::XMFLOAT2 GetFrustumSize(float zoom_factor) {
    float inv_zoom_factor = 1.0f / zoom_factor;
    if (m_width_ >= m_height_) {
      auto aspect_ratio = static_cast<float>(m_width_) / static_cast<float>(m_height_);
      auto frustum_height = inv_zoom_factor * 2.0f * m_near_ * m_tg_half_vertical_fov_;
      auto frustum_width = aspect_ratio * frustum_height;
      return DirectX::XMFLOAT2(frustum_width, frustum_height);
    } else {
      auto aspect_ratio_inv = static_cast<float>(m_height_) / static_cast<float>(m_width_);
      auto frustum_width = inv_zoom_factor * 2.0f * m_near_ * m_tg_half_horizontal_fov_;
      auto frustum_height = aspect_ratio_inv * frustum_width;
      return DirectX::XMFLOAT2(frustum_width, frustum_height);
    }
  }

  DirectX::XMVECTOR GetCurrentTranslationVector(const DirectX::XMFLOAT2& frustum_size) {
    auto e = DirectX::XMLoadFloat2(&m_end_point_);
    auto s = DirectX::XMLoadFloat2(&m_start_point_);
    auto v = DirectX::XMVectorSubtract(e, s);

    auto f = DirectX::XMVectorScale(DirectX::XMLoadFloat2(&frustum_size), 0.5f);

    auto t = DirectX::XMVectorMultiply(v, f);

    return t;
  }

  DirectX::XMVECTOR GetPointOnUnitSphere(const DirectX::XMVECTOR& v) {
    auto l = DirectX::XMVector2LengthSq(v);
    auto b = DirectX::XMVectorReplicate(1.0f);
    auto is_in_unit_circle = DirectX::XMVector2LessOrEqual(l, b);
    if (is_in_unit_circle) {
      auto diff = DirectX::XMVectorSqrt(DirectX::XMVectorSubtract(b, l));
      auto mask = DirectX::XMVectorSelectControl(0, 0, 1, 0);
      return DirectX::XMVectorSelect(v, diff, mask);
    } else {
      return DirectX::XMVectorDivide(v, DirectX::XMVectorSqrt(l));
    }
  }

  DirectX::XMVECTOR GetRotationQuaternion() {
    auto s = DirectX::XMLoadFloat2(&m_start_point_);
    s = GetPointOnUnitSphere(s);

    auto e = DirectX::XMLoadFloat2(&m_end_point_);
    e = GetPointOnUnitSphere(e);

    auto axis = DirectX::XMVector3Cross(e, s);
    if (DirectX::XMVector3Equal(axis, DirectX::XMVectorZero())) {
      return DirectX::XMQuaternionIdentity();
    }

    auto angle = DirectX::XMScalarACos(DirectX::XMVectorGetX(DirectX::XMVector3Dot(e, s)));
    return DirectX::XMQuaternionRotationAxis(axis, angle);
  }

  void UpdateViewMatrix(const DirectX::XMVECTOR& c, const DirectX::XMVECTOR& q, const DirectX::XMFLOAT2& frustum_size) {
    auto center = c;
    auto quaterion = q;

    if (m_current_state_ == State::Panning) {
      auto t_vector = GetCurrentTranslationVector(frustum_size);
      center = DirectX::XMVectorSubtract(center, t_vector);
    } else if (m_current_state_ == State::Rotating) {
      auto r = GetRotationQuaternion();
      quaterion = DirectX::XMQuaternionMultiply(quaterion, r);
    }

    DirectX::XMMATRIX r = DirectX::XMMatrixRotationQuaternion(quaterion);
    DirectX::XMMATRIX r_inv = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionInverse(quaterion));

    DirectX::XMMATRIX t = DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorNegate(center));
    DirectX::XMMATRIX t_inv = DirectX::XMMatrixTranslationFromVector(center);

    m_view_matrix_ = r * t;
    m_inverse_view_matrix_ = t_inv * r_inv;
  }

  // Finished product
  DirectX::XMMATRIX m_view_matrix_;
  DirectX::XMMATRIX m_inverse_view_matrix_;

  // View
  DirectX::XMFLOAT3 m_center_;
  DirectX::XMFLOAT4 m_rotation_quaterion_;

  // Projection
  float m_zoom_factor_;
  float m_near_;
  float m_far_;
  float m_tg_half_horizontal_fov_;
  float m_tg_half_vertical_fov_;

  // Viewport
  uint32_t m_width_;
  uint32_t m_height_;

  // State
  State m_current_state_;
  DirectX::XMFLOAT2 m_start_point_;
  DirectX::XMFLOAT2 m_end_point_;
};