#pragma once

#include <cmath>

#include <d3d11.h>

#include <DirectXMath.h>

#include <dxfw/dxfw.h>

namespace Rendering {
namespace Cameras {

enum class TrackballCameraOperation : uint32_t {
  None = 0,
  Panning = 1,
  Rotating = 2,
  Zooming = 3,
  MaxOperations = 4
};

void TrackballCameraUpdate(TrackballCameraOperation desired_state, float frustum_width, float frustum_height,
                           TrackballCameraOperation* current_state, DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                           DirectX::XMFLOAT3* center, DirectX::XMFLOAT4* rotation_quaterion, float* radius,
                           DirectX::XMMATRIX* view_matrix, DirectX::XMMATRIX* view_matrix_inverse_transpose);

class TrackballCamera {
public:
  TrackballCamera()
      : m_current_state_(TrackballCameraOperation::None),
        m_start_point_(0.0f, 0.0f),
        m_end_point_(0.0f, 0.0f),
        m_center_(0.0f, 0.0f, 0.0f),
        m_rotation_quaterion_(0.0f, 0.0f, 0.0f, 1.0f),  // Quaternion identity
        m_radius_(1.0f),
        m_view_matrix_(DirectX::XMMatrixIdentity()),
        m_view_matrix_inverse_transpose_(DirectX::XMMatrixIdentity()),
        m_desired_state_(TrackballCameraOperation::None) {
  }

  ~TrackballCamera() = default;

  TrackballCamera(const TrackballCamera&) = delete;
  TrackballCamera& operator=(const TrackballCamera&) = delete;

  TrackballCamera(TrackballCamera&&) = default;
  TrackballCamera& operator=(TrackballCamera&&) = default;

  void GetLocation(float* x, float* y, float* z) const {
    *x = m_center_.x;
    *y = m_center_.y;
    *z = m_center_.z;
  }

  void SetLocation(float x, float y, float z) {
    m_center_.x = x;
    m_center_.y = y;
    m_center_.z = z;
  }

  float GetRadius() const {
    return m_radius_;
  }

  void SetRadius(float r) {
    m_radius_ = r;
  }

  void LookAt(float from_x, float from_y, float from_z, float at_x, float at_y, float at_z) {
    m_center_.x = at_x;
    m_center_.y = at_y;
    m_center_.z = at_z;

    auto c = DirectX::XMLoadFloat3(&m_center_);
    auto f = DirectX::XMVectorSet(from_x, from_y, from_z, 0);
    auto v = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(f, c));

    auto zAxis = DirectX::XMVectorSet(0, 0, 1, 0);
    auto axis = DirectX::XMVector3Cross(zAxis, v);
    auto angle = DirectX::XMScalarACos(DirectX::XMVectorGetX(DirectX::XMVector3Dot(zAxis, v)));

    DirectX::XMStoreFloat4(&m_rotation_quaterion_, DirectX::XMQuaternionRotationAxis(axis, angle));
  }

  void SetDesiredState(TrackballCameraOperation desired_state) {
    m_desired_state_ = desired_state;
  }

  void SetEndPoint(const DirectX::XMFLOAT2& p) {
    m_end_point_ = p;
  }

  void UpdateMatrices(float frustum_width, float frustum_height) {
    TrackballCameraUpdate(m_desired_state_, frustum_width, frustum_height, &m_current_state_, &m_start_point_, &m_end_point_, &m_center_, &m_rotation_quaterion_, &m_radius_, &m_view_matrix_, &m_view_matrix_inverse_transpose_);
  }

  const DirectX::XMMATRIX& GetViewMatrix() const {
    return m_view_matrix_;
  }

  const DirectX::XMMATRIX& GetViewMatrixInverseTranspose() const {
    return m_view_matrix_inverse_transpose_;
  }

private:
  // Finished product
  DirectX::XMMATRIX m_view_matrix_;
  DirectX::XMMATRIX m_view_matrix_inverse_transpose_;

  // View
  DirectX::XMFLOAT3 m_center_;
  DirectX::XMFLOAT4 m_rotation_quaterion_;
  float m_radius_;

  // State
  TrackballCameraOperation m_current_state_;
  DirectX::XMFLOAT2 m_start_point_;
  DirectX::XMFLOAT2 m_end_point_;
  
  // Extra state
  TrackballCameraOperation m_desired_state_;
};

}  // namespace Cameras
}  // namespace Rendering
