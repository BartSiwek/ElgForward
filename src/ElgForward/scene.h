#pragma once

#include <vector>

#include <d3d11.h>

#include "memory_helpers.h"
#include "perspective_lens.h"
#include "trackball_camera.h"
#include "camera_script.h"
#include "drawable.h"
#include "constant_buffer.h"
#include "structured_buffer.h"
#include "directional_light.h"
#include "spot_light.h"
#include "point_light.h"

struct Transforms {
  DirectX::XMMATRIX ModelMatrix;
  DirectX::XMMATRIX ModelMatrixInverseTranspose;
  DirectX::XMMATRIX ViewMatrix;
  DirectX::XMMATRIX ViewMatrixInverseTranspose;
  DirectX::XMMATRIX ProjectionMatrix;
  DirectX::XMMATRIX ModelViewMatrix;
  DirectX::XMMATRIX ModelViewMatrixInverseTranspose;
  DirectX::XMMATRIX ModelViewProjectionMatrix;
};

struct LightData {
  int DirectionalLightCount = 0;
  int SpotLightCount = 0;
  int PointLightCount = 0;
  PAD(4);
};

struct Scene {
  std::vector<Drawable> Drawables;
  PerspectiveLens Lens;
  TrackballCamera Camera;
  CameraScript CameraScript;

  Rendering::ConstantBuffer::Handle TransformsConstantBuffer;
  Rendering::ConstantBuffer::Handle LightDataConstantBuffer;
  Rendering::StructuredBuffer::Handle DirectionalLightsStructuredBuffer;
  Rendering::StructuredBuffer::Handle SpotLightsStructuredBuffer;
  Rendering::StructuredBuffer::Handle PointLightsStructuredBuffer;
};
