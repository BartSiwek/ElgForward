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

  void* operator new(size_t size) {
    return aligned_new<Transforms>(size);
  }

  void operator delete(void* ptr) {
    return aligned_delete(ptr);
  }

  void* operator new[](size_t size) {
    return aligned_new_array<Transforms>(size);
  }

  void operator delete[](void* ptr) {
    return aligned_delete_array(ptr);
  }
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

  ConstantBuffer::ConstantBufferHandle TransformsConstantBuffer;
  ConstantBuffer::ConstantBufferHandle LightDataConstantBuffer;
  StructuredBuffer::StructuredBufferHandle DirectionalLightsStructuredBuffer;
  StructuredBuffer::StructuredBufferHandle SpotLightsStructuredBuffer;
  StructuredBuffer::StructuredBufferHandle PointLightsStructuredBuffer;
};
