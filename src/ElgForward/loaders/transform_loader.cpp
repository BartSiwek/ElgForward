#include "transform_loader.h"

#include <DirectXMath.h>

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include <dxfw/dxfw.h>

#include "core/json_helpers.h"
#include "rendering/constant_buffer.h"
#include "rendering/typed_constant_buffer.h"
#include "rendering/transform.h"
#include "rendering/transform_and_inverse_transpose.h"

using namespace Rendering;

namespace Loaders {

bool ReadTransform(const std::string& parent_name, const nlohmann::json& json_transform, ID3D11Device* device, Rendering::Transform::Transform* transform) {
  // Translation
  DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();
  DirectX::XMMATRIX translation_inverse = DirectX::XMMatrixIdentity();

  const auto& json_translation = json_transform["translation"];

  float translation_values[3];
  if (Core::ReadFloat3(json_translation, translation_values)) {
    translation = DirectX::XMMatrixTranslation(translation_values[0], translation_values[1], translation_values[2]);
    translation_inverse = DirectX::XMMatrixTranslation(-translation_values[0], -translation_values[1], -translation_values[2]);
  } else {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid drawable translation %S", json_translation.dump().c_str());
  }

  // Rotation
  DirectX::XMMATRIX rotation = DirectX::XMMatrixIdentity();

  const auto& json_rotation = json_transform["rotation"];

  float rotation_values[4];
  if (Core::ReadFloat3(json_rotation, rotation_values)) {
    rotation = DirectX::XMMatrixRotationRollPitchYaw(rotation_values[0], rotation_values[1], rotation_values[2]);
  } else if (Core::ReadFloat4(json_rotation, rotation_values)) {
    auto axis = DirectX::XMVectorSet(rotation_values[0], rotation_values[1], rotation_values[2], 0.0f);
    rotation = DirectX::XMMatrixRotationAxis(axis, DirectX::XMConvertToRadians(rotation_values[3]));
  } else {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid drawable rotation %S", json_rotation.dump().c_str());
  }

  // Scaling
  DirectX::XMMATRIX scaling = DirectX::XMMatrixIdentity();
  DirectX::XMMATRIX scaling_inverse = DirectX::XMMatrixIdentity();

  const auto& json_scale = json_transform["scale"];

  float scale_values[3];
  if (Core::ReadFloat3(json_scale, scale_values)) {
    scaling = DirectX::XMMatrixScaling(scale_values[0], scale_values[1], scale_values[2]);
    scaling_inverse = DirectX::XMMatrixScaling(1.0f / scale_values[0], 1.0f / scale_values[1], 1.0f / scale_values[2]);
  } else {
    DXFW_TRACE(__FILE__, __LINE__, false, "Invalid drawable scaling %S", json_scale.dump().c_str());
  }

  // Transform
  Rendering::Transform::TransformAndInverseTranspose transform_data;

  transform_data.Matrix = scaling * rotation * translation;
  transform_data.MatrixInverseTranspose = scaling_inverse * rotation * DirectX::XMMatrixTranspose(translation_inverse);

  // Constant buffer
  std::string name = parent_name + " transform";
  auto transform_constant_buffer = Rendering::ConstantBuffer::Create(name, &transform_data, device);
  if (!transform_constant_buffer.IsValid()) {
    return false;
  }

  transform->TransformConstantBuffer = static_cast<Rendering::ConstantBuffer::Handle>(transform_constant_buffer);

  return true;
}

}  // namespace Loaders
