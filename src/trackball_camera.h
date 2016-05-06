#pragma once

#include <cmath>

#include <d3d11.h>

#include <DirectXMath.h>

#include <dxfw/dxfw.h>

enum class TrackballCameraOperation : uint32_t {
  None = 0,
  Panning = 1,
  Rotating = 2,
  MaxOperations = 3
};

void TrackballCameraUpdate(TrackballCameraOperation desired_state, const DirectX::XMFLOAT2& p, float frustum_width, float frustum_height,
                           TrackballCameraOperation* current_state, DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                           DirectX::XMFLOAT3* center, DirectX::XMFLOAT4* rotation_quaterion, float* radius,
                           DirectX::XMMATRIX* view_matrix, DirectX::XMMATRIX* inverse_view_matrix);

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
        m_inverse_view_matrix_(DirectX::XMMatrixIdentity()),
        m_desired_state_(TrackballCameraOperation::None),
        m_p_(0.0f, 0.0f) {
  }

  ~TrackballCamera() = default;

  TrackballCamera(const TrackballCamera&) = delete;
  TrackballCamera& operator=(const TrackballCamera&) = delete;

  TrackballCamera(TrackballCamera&&) = default;
  TrackballCamera& operator=(TrackballCamera&&) = default;

  void SetLocation(float x, float y, float z) {
    m_center_.x = x;
    m_center_.y = y;
    m_center_.z = z;
  }

  void SetRadius(float r) {
    m_radius_ = r;
  }

  void LookAt(float /* x */, float /* y */, float /* z */) {
    /*
    FIX ME
    auto zAxis = DirectX::XMVectorSet(0, 0, 1, 0);
    auto c = DirectX::XMLoadFloat3(&m_center_);
    auto v = DirectX::XMPlaneNormalize(DirectX::XMVectorSubtract(DirectX::XMVectorSet(x, y, z, 0), c));

    auto axis = DirectX::XMVector3Cross(zAxis, v);
    auto angle = DirectX::XMScalarACos(DirectX::XMVectorGetX(DirectX::XMVector3Dot(zAxis, v)));

    DirectX::XMStoreFloat4(&m_rotation_quaterion_, DirectX::XMQuaternionRotationAxis(axis, angle));
    */
  }

  void SetDesiredState(TrackballCameraOperation desired_state) {
    m_desired_state_ = desired_state;
  }

  void SetEndPoint(const DirectX::XMFLOAT2& p) {
    m_p_ = p;
  }

  void UpdateMatrices(float frustum_width, float frustum_height) {
    TrackballCameraUpdate(m_desired_state_, m_p_, frustum_width, frustum_height, &m_current_state_, &m_start_point_, &m_end_point_, &m_center_, &m_rotation_quaterion_, &m_radius_, &m_view_matrix_, &m_inverse_view_matrix_);
  }

  const DirectX::XMMATRIX& GetViewMatrix() const {
    return m_view_matrix_;
  }

  const DirectX::XMMATRIX& GetViewMatrixInverse() const {
    return m_inverse_view_matrix_;
  }

private:
  // Finished product
  DirectX::XMMATRIX m_view_matrix_;
  DirectX::XMMATRIX m_inverse_view_matrix_;

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
  DirectX::XMFLOAT2 m_p_;
};