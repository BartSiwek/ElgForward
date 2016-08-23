#pragma once

#include <vector>

#include <d3d11.h>

#include "core/memory_helpers.h"
#include "rendering/lens/perspective_lens.h"
#include "rendering/cameras/trackball_camera.h"
#include "rendering/lights/directional_light.h"
#include "rendering/lights/point_light.h"
#include "rendering/lights/spot_light.h"
#include "rendering/camera_script.h"
#include "rendering/drawable.h"
#include "rendering/typed_constant_buffer.h"
#include "rendering/typed_structured_buffer.h"

struct PerFrame {
  int DirectionalLightCount = 0;
  int SpotLightCount = 0;
  int PointLightCount = 0;
  PAD(4);
};

struct PerCamera {
  DirectX::XMMATRIX ViewMatrix;
  DirectX::XMMATRIX ViewMatrixInverseTranspose;
  DirectX::XMMATRIX ProjectionMatrix;
};

struct Scene {
  std::vector<Rendering::Drawable> Drawables;
  Rendering::Lens::PerspectiveLens Lens;
  Rendering::Cameras::TrackballCamera Camera;
  Rendering::CameraScript CameraScript;

  Rendering::ConstantBuffer::TypedHandle<PerFrame> PerFrameConstantBuffer;
  Rendering::ConstantBuffer::TypedHandle<PerCamera> PerCameraConstantBuffer;
  Rendering::StructuredBuffer::TypedHandle<Rendering::Lights::DirectionalLight> DirectionalLightsStructuredBuffer;
  Rendering::StructuredBuffer::TypedHandle<Rendering::Lights::SpotLight> SpotLightsStructuredBuffer;
  Rendering::StructuredBuffer::TypedHandle<Rendering::Lights::PointLight> PointLightsStructuredBuffer;

  Microsoft::WRL::ComPtr<ID3D11Texture2D > Texture;
};
