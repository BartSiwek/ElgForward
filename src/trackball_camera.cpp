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
void StartOperation(TrackballCameraOperation operation, TrackballCameraOperation* current_state, DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point) {
  *current_state = operation;
  *start_point = *end_point;
}

void EndOperation(TrackballCameraOperation* current_state, DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point) {
  *current_state = TrackballCameraOperation::None;
  start_point->x = start_point->y = 0.0f;
  end_point->x = end_point->y = 0.0f;
}

/* STATE TRANSITION HANDLERS */
void HandleNoneToNoneTransition(const DirectX::XMVECTOR& /* frustum_size */, TrackballCameraOperation* /* current_state */,
                                DirectX::XMFLOAT2* /* start_point */, DirectX::XMFLOAT2* /* end_point */,
                                DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */,
                                DirectX::XMVECTOR* /* center */, DirectX::XMVECTOR* /* rotation_quaterion */) {
}

void HandleNoneToPanningTransition(const DirectX::XMVECTOR& /* frustum_size */, TrackballCameraOperation* current_state,
                                   DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                                   DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */,
                                   DirectX::XMVECTOR* /* center */, DirectX::XMVECTOR* /* rotation_quaterion */) {
  StartOperation(TrackballCameraOperation::Panning, current_state, start_point, end_point);
}

void HandleNoneToRotatingTransition(const DirectX::XMVECTOR& /* frustum_size */, TrackballCameraOperation* current_state,
                                    DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                                    DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */,
                                    DirectX::XMVECTOR* /* center */, DirectX::XMVECTOR* /* rotation_quaterion */) {
  StartOperation(TrackballCameraOperation::Rotating, current_state, start_point, end_point);
}

void HandlePanningToNoneTransition(const DirectX::XMVECTOR& frustum_size, TrackballCameraOperation* current_state,
                                   DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                                   DirectX::XMFLOAT3* center_storage, DirectX::XMFLOAT4* /* rotation_quaterion_storage */,
                                   DirectX::XMVECTOR* center, DirectX::XMVECTOR* rotation_quaterion) {
  *center = GetCurrentTranslationVector(*start_point, *end_point, *center, *rotation_quaterion, frustum_size);
  DirectX::XMStoreFloat3(center_storage, *center);
  EndOperation(current_state, start_point, end_point);
}

void HandlePanningToPanningTransition(const DirectX::XMVECTOR& frustum_size, TrackballCameraOperation* /* current_state */,
                                      DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                                      DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */,
                                      DirectX::XMVECTOR* center, DirectX::XMVECTOR* rotation_quaterion) {
  *center = GetCurrentTranslationVector(*start_point, *end_point, *center, *rotation_quaterion, frustum_size);
}

void HandlePanningToRotatingTransition(const DirectX::XMVECTOR& frustum_size, TrackballCameraOperation* /* current_state */,
                                       DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                                       DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */,
                                       DirectX::XMVECTOR* center, DirectX::XMVECTOR* rotation_quaterion) {
  *center = GetCurrentTranslationVector(*start_point, *end_point, *center, *rotation_quaterion, frustum_size);
}

void HandleRotatingToNoneTransition(const DirectX::XMVECTOR& /* frustum_size */, TrackballCameraOperation* current_state,
                                    DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                                    DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* rotation_quaterion_storage,
                                    DirectX::XMVECTOR* /* center */, DirectX::XMVECTOR* rotation_quaterion) {
  *rotation_quaterion = GetCurrentRotationQuaternion(*start_point, *end_point, *rotation_quaterion);
  DirectX::XMStoreFloat4(rotation_quaterion_storage, *rotation_quaterion);
  EndOperation(current_state, start_point, end_point);
}

void HandleRotatingToPanningTransition(const DirectX::XMVECTOR& /* frustum_size */, TrackballCameraOperation* /* current_state */,
                                       DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                                       DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */,
                                       DirectX::XMVECTOR* /* center */, DirectX::XMVECTOR* rotation_quaterion) {
  *rotation_quaterion = GetCurrentRotationQuaternion(*start_point, *end_point, *rotation_quaterion);
}

void HandleRotatingToRotatingTransition(const DirectX::XMVECTOR& /* frustum_size */, TrackballCameraOperation* /* current_state */,
                                        DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                                        DirectX::XMFLOAT3* /* center_storage */, DirectX::XMFLOAT4* /* rotation_quaterion_storage */,
                                        DirectX::XMVECTOR* /* center */, DirectX::XMVECTOR* rotation_quaterion) {
  *rotation_quaterion = GetCurrentRotationQuaternion(*start_point, *end_point, *rotation_quaterion);
}

/* STATE TRANSITION HANDLER TABLE */
using TransitionHandler = void(*)(const DirectX::XMVECTOR& frustum_size, TrackballCameraOperation* current_state,
                                  DirectX::XMFLOAT2* start_point, DirectX::XMFLOAT2* end_point,
                                  DirectX::XMFLOAT3* center_storage, DirectX::XMFLOAT4* rotation_quaterion_storage,
                                  DirectX::XMVECTOR* center, DirectX::XMVECTOR* rotation_quaterion);

using TransitionHandlerTable = std::array<std::array<TransitionHandler, static_cast<size_t>(TrackballCameraOperation::MaxOperations)>, static_cast<size_t>(TrackballCameraOperation::MaxOperations)>;

constexpr TransitionHandlerTable g_transition_table_ = { 
  // None
  {
    HandleNoneToNoneTransition,
    HandleNoneToPanningTransition,
    HandleNoneToRotatingTransition,
    HandlePanningToNoneTransition,
    HandlePanningToPanningTransition,
    HandlePanningToRotatingTransition,
    HandleRotatingToNoneTransition,
    HandleRotatingToPanningTransition,
    HandleRotatingToRotatingTransition,
  }
};

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
  (*handler)(frustum_size_v, current_state, start_point, end_point, center, rotation_quaterion, &center_v, &rotation_quaterion_v);

  UpdateViewMatrix(center_v, rotation_quaterion_v, radius_f, view_matrix, inverse_view_matrix);
}