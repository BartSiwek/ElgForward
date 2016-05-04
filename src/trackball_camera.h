#pragma once

#include <cmath>

#include <d3d11.h>

#include <DirectXMath.h>

#include <dxfw/dxfw.h>

enum class TrackballCameraOperation {
  None,
  Panning,
  Rotating,
};

inline DirectX::XMFLOAT2 GetNormalizedScreenCoordinates(float width, float height, float x, float y) {
  return DirectX::XMFLOAT2((2 * x) / width - 1.0f, 1.0f - (2 * y) / height);

}

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
        m_inverse_view_matrix_(DirectX::XMMatrixIdentity()) {
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

  void UpdateMatrices(TrackballCameraOperation desired_state, float frustum_width, float frustum_height, const DirectX::XMFLOAT2& p) {
    auto frustum_size_v = DirectX::XMVectorSet(frustum_width, frustum_height, 0.0f, 0.0f);
    auto center = DirectX::XMLoadFloat3(&m_center_);
    auto q = DirectX::XMLoadFloat4(&m_rotation_quaterion_);

    if (desired_state == TrackballCameraOperation::Panning && m_current_state_ == TrackballCameraOperation::None) {
      StartOperation(TrackballCameraOperation::Panning, p);
    } else if (desired_state == TrackballCameraOperation::Panning && m_current_state_ == TrackballCameraOperation::Panning) {
      m_end_point_ = p;
    } else if (desired_state == TrackballCameraOperation::Rotating && m_current_state_ == TrackballCameraOperation::None) {
      StartOperation(TrackballCameraOperation::Rotating, p);
    } else if (desired_state == TrackballCameraOperation::Rotating && m_current_state_ == TrackballCameraOperation::Rotating) {
      m_end_point_ = p;
    } else if (desired_state == TrackballCameraOperation::None && m_current_state_ == TrackballCameraOperation::Panning) {
      center = DirectX::XMVectorSubtract(center, GetCurrentTranslationVector(q, frustum_size_v));
      DirectX::XMStoreFloat3(&m_center_, center);
      EndOperation();
    } else if (desired_state == TrackballCameraOperation::None && m_current_state_ == TrackballCameraOperation::Rotating) {
      q = DirectX::XMQuaternionMultiply(q, GetRotationQuaternion());
      DirectX::XMStoreFloat4(&m_rotation_quaterion_, q);
      EndOperation();
    }

    UpdateViewMatrix(center, q, frustum_size_v);
  }

  const DirectX::XMMATRIX& GetViewMatrix() const {
    return m_view_matrix_;
  }

  const DirectX::XMMATRIX& GetViewMatrixInverse() const {
    return m_inverse_view_matrix_;
  }

private:
  void StartOperation(TrackballCameraOperation operation, const DirectX::XMFLOAT2& p) {
    if (m_current_state_ == TrackballCameraOperation::None) {
      m_current_state_ = operation;
      m_start_point_ = p;
      m_end_point_ = m_start_point_;
    }
  }

  void EndOperation() {
    if (m_current_state_ != TrackballCameraOperation::None) {
      m_current_state_ = TrackballCameraOperation::None;
      m_start_point_.x = m_start_point_.y = 0.0f;
      m_end_point_.x = m_end_point_.y = 0.0f;
    }
  }

  DirectX::XMVECTOR GetCurrentTranslationVector(const DirectX::XMVECTOR& q, const DirectX::XMVECTOR& frustum_size) {
    auto e = DirectX::XMLoadFloat2(&m_end_point_);
    auto s = DirectX::XMLoadFloat2(&m_start_point_);
    auto v = DirectX::XMVectorSubtract(e, s);

    auto f = DirectX::XMVectorScale(frustum_size, 0.5f);

    auto t = DirectX::XMVectorMultiply(v, f);
    
    t = DirectX::XMVector3Rotate(t, DirectX::XMQuaternionInverse(q));

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

  void UpdateViewMatrix(const DirectX::XMVECTOR& c, const DirectX::XMVECTOR& q, const DirectX::XMVECTOR& frustum_size) {
    auto center = c;
    auto quaterion = q;

    if (m_current_state_ == TrackballCameraOperation::Panning) {
      auto t_vector = GetCurrentTranslationVector(quaterion, frustum_size);
      center = DirectX::XMVectorSubtract(center, t_vector);
    } else if (m_current_state_ == TrackballCameraOperation::Rotating) {
      auto r = GetRotationQuaternion();
      quaterion = DirectX::XMQuaternionMultiply(quaterion, r);
    }

    DirectX::XMMATRIX t = DirectX::XMMatrixTranslation(0, 0, m_radius_);
    DirectX::XMMATRIX t_inv = DirectX::XMMatrixTranslation(0, 0, -m_radius_);

    DirectX::XMMATRIX r = DirectX::XMMatrixRotationQuaternion(quaterion);
    DirectX::XMMATRIX r_inv = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionInverse(quaterion));

    DirectX::XMMATRIX c_t = DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorNegate(center));
    DirectX::XMMATRIX c_t_inv = DirectX::XMMatrixTranslationFromVector(center);

    m_view_matrix_ = c_t * r * t;
    m_inverse_view_matrix_ = t_inv * r_inv * c_t_inv;
  }

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
};