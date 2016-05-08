#include "trackball_camera.h"

#include <array>

#include <DirectXMath.h>

/* MATH HELPERS */
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

DirectX::XMVECTOR GetCurrentTranslationVector(const DirectX::XMFLOAT2& start_point, const DirectX::XMFLOAT2& end_point, const DirectX::XMVECTOR& center, const DirectX::XMVECTOR& q, const DirectX::XMVECTOR& frustum_size) {
  auto e = DirectX::XMLoadFloat2(&end_point);
  auto s = DirectX::XMLoadFloat2(&start_point);
  auto v = DirectX::XMVectorSubtract(e, s);

  auto f = DirectX::XMVectorScale(frustum_size, 0.5f);

  auto t = DirectX::XMVectorMultiply(v, f);

  t = DirectX::XMVector3Rotate(t, DirectX::XMQuaternionInverse(q));

  t = DirectX::XMVectorSubtract(center, t);

  return t;
}

DirectX::XMVECTOR GetCurrentRotationQuaternion(const DirectX::XMFLOAT2& start_point, const DirectX::XMFLOAT2& end_point, const DirectX::XMVECTOR& rotation_quaterion) {
  auto s = DirectX::XMLoadFloat2(&start_point);
  s = GetPointOnUnitSphere(s);

  auto e = DirectX::XMLoadFloat2(&end_point);
  e = GetPointOnUnitSphere(e);

  auto axis = DirectX::XMVector3Cross(e, s);
  if (DirectX::XMVector3Equal(axis, DirectX::XMVectorZero())) {
    return rotation_quaterion;
  }

  auto angle = DirectX::XMScalarACos(DirectX::XMVectorGetX(DirectX::XMVector3Dot(e, s)));

  auto q = DirectX::XMQuaternionRotationAxis(axis, angle);

  q = DirectX::XMQuaternionMultiply(rotation_quaterion, q);

  return q;
}

float GetCurrentRadius(const DirectX::XMFLOAT2& start_point, const DirectX::XMFLOAT2& end_point, float radius) {
  float delta = end_point.y - start_point.y;
  delta = (-3.0f * delta + 5.0f) / 4.0f;
  return delta * radius;
}

void UpdateViewMatrix(const DirectX::XMVECTOR& center, const DirectX::XMVECTOR& rotation_quaterion, float radius, DirectX::XMMATRIX* view_matrix, DirectX::XMMATRIX* inverse_view_matrix) {
  DirectX::XMMATRIX t = DirectX::XMMatrixTranslation(0, 0, radius);
  DirectX::XMMATRIX t_inv = DirectX::XMMatrixTranslation(0, 0, -radius);

  DirectX::XMMATRIX r = DirectX::XMMatrixRotationQuaternion(rotation_quaterion);
  DirectX::XMMATRIX r_inv = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionInverse(rotation_quaterion));

  DirectX::XMMATRIX c_t = DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorNegate(center));
  DirectX::XMMATRIX c_t_inv = DirectX::XMMatrixTranslationFromVector(center);

  *view_matrix = c_t * r * t;
  *inverse_view_matrix = t_inv * r_inv * c_t_inv;
}

/* STATE SWITCHING HELPERS */
void EndOperation(TrackballCameraOperation* current_state, DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point) {
  *current_state = TrackballCameraOperation::None;
  start_point->x = start_point->y = 0.0f;
  end_point->x = end_point->y = 0.0f;
}

/* STATE TRANSITION HANDLERS */
void NullTransition(const DirectX::XMVECTOR& /* frustum_size */, TrackballCameraOperation* /* current_state */,
                    DirectX::XMFLOAT2* /* start_point */, DirectX::XMFLOAT2* /* end_point */,
                    DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */, float* /* radius_storage */,
                    DirectX::XMVECTOR* /* center */, DirectX::XMVECTOR* /* rotation_quaterion */, float* /* radius */) {
}

template<TrackballCameraOperation Operation>
void StartOperationTransition(const DirectX::XMVECTOR& /* frustum_size */, TrackballCameraOperation* current_state,
                              DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                              DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */, float* /* radius_storage */,
                              DirectX::XMVECTOR* /* center */, DirectX::XMVECTOR* /* rotation_quaterion */, float* /* radius */) {
  *current_state = Operation;
  *start_point = *end_point;
}

void PanningUpdate(const DirectX::XMVECTOR& frustum_size, TrackballCameraOperation* /* current_state */,
                   DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                   DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */, float* /* radius_storage */,
                   DirectX::XMVECTOR* center, DirectX::XMVECTOR* rotation_quaterion, float* /* radius */) {
  *center = GetCurrentTranslationVector(*start_point, *end_point, *center, *rotation_quaterion, frustum_size);
}

void RotatingUpdate(const DirectX::XMVECTOR& /* frustum_size */, TrackballCameraOperation* /* current_state */,
                    DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                    DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */, float* /* radius_storage */,
                    DirectX::XMVECTOR* /* center */, DirectX::XMVECTOR* rotation_quaterion, float* /* radius */) {
  *rotation_quaterion = GetCurrentRotationQuaternion(*start_point, *end_point, *rotation_quaterion);
}

void ZoomingUpdate(const DirectX::XMVECTOR& /* frustum_size */, TrackballCameraOperation* /* current_state */,
                   DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                   DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */, float* /* radius_storage */,
                   DirectX::XMVECTOR* /* center */, DirectX::XMVECTOR* /* rotation_quaterion */, float* radius) {
  *radius = GetCurrentRadius(*start_point, *end_point, *radius);
}

void EndPanning(const DirectX::XMVECTOR& frustum_size, TrackballCameraOperation* current_state,
                DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                DirectX::XMFLOAT3* center_storage, DirectX::XMFLOAT4* /* rotation_quaterion_storage */, float* /* radius_storage */,
                DirectX::XMVECTOR* center, DirectX::XMVECTOR* rotation_quaterion, float* /* radius */) {
  *center = GetCurrentTranslationVector(*start_point, *end_point, *center, *rotation_quaterion, frustum_size);
  DirectX::XMStoreFloat3(center_storage, *center);
  EndOperation(current_state, start_point, end_point);
}

void EndRotating(const DirectX::XMVECTOR& /* frustum_size */, TrackballCameraOperation* current_state,
                 DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                 DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* rotation_quaterion_storage, float* /* radius_storage */,
                 DirectX::XMVECTOR* /* center */, DirectX::XMVECTOR* rotation_quaterion, float* /* radius */) {
  *rotation_quaterion = GetCurrentRotationQuaternion(*start_point, *end_point, *rotation_quaterion);
  DirectX::XMStoreFloat4(rotation_quaterion_storage, *rotation_quaterion);
  EndOperation(current_state, start_point, end_point);
}

void EndZooming(const DirectX::XMVECTOR& /* frustum_size */, TrackballCameraOperation* current_state,
                DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */, float* radius_storage,
                DirectX::XMVECTOR* /* center */, DirectX::XMVECTOR* /* rotation_quaterion */, float* radius) {
  *radius = GetCurrentRadius(*start_point, *end_point, *radius);
  *radius_storage = *radius;
  EndOperation(current_state, start_point, end_point);
}

/* STATE TRANSITION HANDLER TABLE */
using TransitionHandler = void(*)(const DirectX::XMVECTOR& frustum_size, TrackballCameraOperation* current_state,
                                  DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                                  DirectX::XMFLOAT3* center_storage, DirectX::XMFLOAT4* rotation_quaterion_storage, float* radius_storage,
                                  DirectX::XMVECTOR* center, DirectX::XMVECTOR* rotation_quaterion, float* radius);

using TransitionHandlerTable = std::array<std::array<TransitionHandler, static_cast<size_t>(TrackballCameraOperation::MaxOperations)>, static_cast<size_t>(TrackballCameraOperation::MaxOperations)>;

constexpr TransitionHandlerTable g_transition_table_ = {{
  // None
  {
    // None -> None
    NullTransition,
    // None -> Panning
    StartOperationTransition<TrackballCameraOperation::Panning>,
    // None -> Rotating
    StartOperationTransition<TrackballCameraOperation::Rotating>,
    // None -> Zooming
    StartOperationTransition<TrackballCameraOperation::Zooming>,
  },
  // Panning
  {
    // Panning -> None
    EndPanning,
    // Panning -> Panning
    PanningUpdate,
    // Panning -> Rotating
    PanningUpdate,
    // Panning -> Zooming
    PanningUpdate,
  },
  // Rotating
  {
    // Rotating -> None
    EndRotating,
    // Rotating -> Panning
    RotatingUpdate,
    // Rotating -> Rotating
    RotatingUpdate,
    // Rotating -> Zooming
    RotatingUpdate,
  },
  // Zooming
  {
    // Zooming -> None
    EndZooming,
    // Zooming -> Panning
    ZoomingUpdate,
    // Zooming -> Rotating
    ZoomingUpdate,
    // Zooming -> Zooming
    ZoomingUpdate,
  },
}};

/* MAIN UPDATE ROUTINE */
void TrackballCameraUpdate(TrackballCameraOperation desired_state, float frustum_width, float frustum_height,
                           TrackballCameraOperation* current_state, DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                           DirectX::XMFLOAT3* center, DirectX::XMFLOAT4* rotation_quaterion, float* radius,
                           DirectX::XMMATRIX* view_matrix, DirectX::XMMATRIX* inverse_view_matrix) {
  auto frustum_size_v = DirectX::XMVectorSet(frustum_width, frustum_height, 0.0f, 0.0f);

  auto center_v = DirectX::XMLoadFloat3(center);
  auto rotation_quaterion_v = DirectX::XMLoadFloat4(rotation_quaterion);
  auto radius_f = *radius;

  auto handler = g_transition_table_[static_cast<uint32_t>(*current_state)][static_cast<uint32_t>(desired_state)];
  (*handler)(frustum_size_v, current_state, start_point, end_point, center, rotation_quaterion, radius, &center_v, &rotation_quaterion_v, &radius_f);

  UpdateViewMatrix(center_v, rotation_quaterion_v, radius_f, view_matrix, inverse_view_matrix);
}
