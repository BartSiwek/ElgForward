#include <string>
#include <memory>
#include <sstream>
#include <iostream>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <D3Dcompiler.h>
#include <wrl.h>
#endif

#include "filesystem.h"
#include "dxfw_wrapper.h"
#include "dxfw_helpers.h"
#include "directx_state.h"
#include "scene.h"
#include "constant_buffer.h"
#include "mesh.h"
#include "vertex_layout.h"
#include "material.h"
#include "drawable.h"
#include "screen.h"
#include "perspective_lens.h"
#include "trackball_camera.h"
#include "scene_loader.h"

bool InitializeDeviceAndSwapChain(DirectXState* state) {
  // Device settings
  UINT create_device_flags = 0;
#ifdef _DEBUG
  create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_DRIVER_TYPE driver_types[] = {
    D3D_DRIVER_TYPE_HARDWARE,
  };
  UINT num_driver_types = 1;

  D3D_FEATURE_LEVEL feature_levels[] = {
    D3D_FEATURE_LEVEL_11_0
  };
  UINT num_feature_levels = 1;

  // SwapChain settings
  DXGI_SWAP_CHAIN_DESC swap_chain_desc; 
  ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));

  HWND window_handle = Dxfw::GetWindowHandle(state->window.get());
  uint32_t width;
  uint32_t height;
  Dxfw::GetWindowSize(state->window.get(), &width, &height);

  swap_chain_desc.BufferCount = 1;
  swap_chain_desc.BufferDesc.Width = width;
  swap_chain_desc.BufferDesc.Height = height;
  swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
  swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
  swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.OutputWindow = window_handle;
  swap_chain_desc.SampleDesc.Count = 1;
  swap_chain_desc.SampleDesc.Quality = 0;
  swap_chain_desc.Windowed = TRUE;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  // Create
  auto hr = S_OK;
  for (decltype(num_driver_types) driver_type_index = 0; driver_type_index < num_driver_types; driver_type_index++) {
    D3D_DRIVER_TYPE driver_type = driver_types[driver_type_index];
    D3D_FEATURE_LEVEL result_feature_level;
    hr = D3D11CreateDeviceAndSwapChain(nullptr, driver_type, nullptr, create_device_flags, feature_levels,
                                       num_feature_levels, D3D11_SDK_VERSION, &swap_chain_desc,
                                       state->swap_chain.GetAddressOf(), state->device.GetAddressOf(),
                                       &result_feature_level, state->device_context.GetAddressOf());

    if (SUCCEEDED(hr)) {
      break;
    }

    if (FAILED(hr)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, false, hr);
    }
  }

  if (FAILED(hr)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, hr);
    return false;
  }

  return true;
}

bool InitializeRenderTarget(DirectXState* state) {
  // Create our BackBuffer
  ID3D11Texture2D* back_buffer;
  auto back_buffer_result = state->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));
  if (FAILED(back_buffer_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, back_buffer_result);
    return false;
  }

  // Create our Render Target
  auto rtv_result = state->device->CreateRenderTargetView(back_buffer, NULL, state->render_target_view.GetAddressOf());
  if (FAILED(rtv_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, rtv_result);
    return false;
  }

  // Back buffer is held by RTV, so we can release it here
  back_buffer->Release();

  // Set our Render Target
  state->device_context->OMSetRenderTargets(1, state->render_target_view.GetAddressOf(), NULL);

  return true;
}

bool InitializeDirect3d11(DirectXState* state) {
  // Crate window
  state->window.reset(Dxfw::CreateNewWindow(800, 600, "Hello DirectX"));
  if (!state->window) {
    return false;
  }

  // Create device
  bool device_ok = InitializeDeviceAndSwapChain(state);
  if (!device_ok) {
    return false;
  }

  // Create RT
  bool rt_ok = InitializeRenderTarget(state);
  if (!rt_ok) {
    return false;
  }

  // Set the viewport
  uint32_t width;
  uint32_t height;
  Dxfw::GetWindowSize(state->window.get(), &width, &height);

  SetViewportSize(&state->viewport, width, height);
  state->device_context->RSSetViewports(1, &state->viewport);

  // Create the resize callback
  Dxfw::RegisterWindowResizeCallback(state->window.get(), [state](dxfwWindow* /* window */, uint32_t width, uint32_t height) {
    state->device_context->OMSetRenderTargets(0, 0, 0);

    // Release all outstanding references to the swap chain's buffers.
    state->render_target_view.Reset();

    // Preserve the existing buffer count and format and automatically choose the width and height to match the client rect for HWNDs.
    auto resize_buffers_result = state->swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

    if (FAILED(resize_buffers_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, resize_buffers_result);
      return;
    }

    bool rt_ok = InitializeRenderTarget(state);
    if (!rt_ok) {
      return;
    }

    // Set viewport data
    SetViewportSize(&state->viewport, width, height);
    state->device_context->RSSetViewports(1, &state->viewport);
  });

  return true;
}

bool InitializeScene(DirectXState* state, Scene* scene) {
  scene->TransformsConstantBuffer = ConstantBuffer::Create<Transforms>("Transforms", nullptr, state->device.Get());
  if (!scene->TransformsConstantBuffer.IsValid()) {
    return false;
  }

  scene->LightDataConstantBuffer = ConstantBuffer::Create<LightData>("LightData", nullptr, state->device.Get());
  if (!scene->TransformsConstantBuffer.IsValid()) {
    return false;
  }

  scene->DirectionalLightsStructuredBuffer = StructuredBuffer::Create<DirectionalLight>("DirectionalLights", 1000, nullptr, 0, state->device.Get());
  if (!scene->DirectionalLightsStructuredBuffer.IsValid()) {
    return false;
  }

  scene->SpotLightsStructuredBuffer = StructuredBuffer::Create<SpotLight>("SpotLights", 1000, nullptr, 0, state->device.Get());
  if (!scene->SpotLightsStructuredBuffer.IsValid()) {
    return false;
  }

  scene->PointLightsStructuredBuffer = StructuredBuffer::Create<PointLight>("PointLights", 1000, nullptr, 0, state->device.Get());
  if (!scene->PointLightsStructuredBuffer.IsValid()) {
    return false;
  }

  return true;
}

void UpdateDrawableBuffers(const Drawable& drawable, Scene* scene, DirectXState* state) {
  // Transforms
  auto buffer = ConstantBuffer::GetCpuBuffer<Transforms>(scene->TransformsConstantBuffer);
  buffer->ModelMatrix = drawable.GetModelMatrix();
  buffer->ModelMatrixInverseTranspose = drawable.GetModelMatrixInverseTranspose();
  buffer->ViewMatrix = scene->Camera.GetViewMatrix();
  buffer->ViewMatrixInverseTranspose = scene->Camera.GetViewMatrixInverseTranspose();
  buffer->ProjectionMatrix = scene->Lens.GetProjectionMatrix();
  buffer->ModelViewMatrix = buffer->ModelMatrix * buffer->ViewMatrix;
  buffer->ModelViewMatrixInverseTranspose = buffer->ModelMatrixInverseTranspose * buffer->ViewMatrixInverseTranspose;
  buffer->ModelViewProjectionMatrix = buffer->ModelViewMatrix * buffer->ProjectionMatrix;

  bool update_ok = SendToGpu(scene->TransformsConstantBuffer, state->device_context.Get());
  if (update_ok) {
    state->device_context->VSSetConstantBuffers(0, 1, ConstantBuffer::GetAddressOfGpuBuffer(scene->TransformsConstantBuffer));
  } else {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error updating per frame constant buffer");
  }

  // Material
}

void Render(Scene* scene, DirectXState* state) {
  float bgColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  state->device_context->ClearRenderTargetView(state->render_target_view.Get(), bgColor);

  for (auto& drawable : scene->Drawables) {
    UpdateDrawableBuffers(drawable, scene, state);

    state->device_context->VSSetShader(drawable.GetVertexShader(), 0, 0);
    state->device_context->PSSetShader(drawable.GetPixelShader(), 0, 0);

    state->device_context->IASetVertexBuffers(0,
                                              D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT,
                                              drawable.GetVertexBuffers(),
                                              drawable.GetVertexBufferStrides(),
                                              drawable.GetVertexBufferOffsets());

    state->device_context->IASetIndexBuffer(drawable.GetIndexBuffer(), drawable.GetIndexBufferFormat(), 0);
    state->device_context->IASetPrimitiveTopology(drawable.GetPrimitiveTopology());

    state->device_context->IASetInputLayout(drawable.GetVertexLayout());

    state->device_context->DrawIndexed(drawable.GetIndexCount(), 0, 0);
  }
}

void UpdateFrameBuffers(Scene* scene, DirectXState* state) {
  // Point lights
  auto point_light_cpu_buffer = StructuredBuffer::GetCpuBuffer<PointLight>(scene->PointLightsStructuredBuffer);
  for (auto point_light = point_light_cpu_buffer,
       point_light_end = point_light_cpu_buffer + StructuredBuffer::GetCurrentSize(scene->PointLightsStructuredBuffer);
       point_light != point_light_end;
       ++point_light) {
    point_light->Update(scene->Camera.GetViewMatrix());
  }
  
  bool point_update_ok = SendToGpu(scene->PointLightsStructuredBuffer, state->device_context.Get());
  if (point_update_ok) {
    state->device_context->VSSetShaderResources(0, 1, GetAddressOfShaderResourceView(scene->PointLightsStructuredBuffer));
  } else {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error updating point light buffer");
  }

  // Spot lights
  auto spot_light_cpu_buffer = StructuredBuffer::GetCpuBuffer<SpotLight>(scene->SpotLightsStructuredBuffer);
  for (auto spot_light = spot_light_cpu_buffer,
       spot_light_end = spot_light_cpu_buffer + StructuredBuffer::GetCurrentSize(scene->SpotLightsStructuredBuffer);
       spot_light != spot_light_end;
       ++spot_light) {
    spot_light->Update(scene->Camera.GetViewMatrix());
  }

  bool spot_update_ok = SendToGpu(scene->SpotLightsStructuredBuffer, state->device_context.Get());
  if (spot_update_ok) {
    state->device_context->VSSetShaderResources(1, 1, GetAddressOfShaderResourceView(scene->SpotLightsStructuredBuffer));
  } else {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error updating spot light buffer");
  }

  // Directional lights
  auto directional_light_cpu_buffer = StructuredBuffer::GetCpuBuffer<DirectionalLight>(scene->DirectionalLightsStructuredBuffer);
  for (auto directional_light = directional_light_cpu_buffer,
       directional_light_end = directional_light_cpu_buffer + StructuredBuffer::GetCurrentSize(scene->DirectionalLightsStructuredBuffer);
       directional_light != directional_light_end;
       ++directional_light) {
    directional_light->Update(scene->Camera.GetViewMatrix());
  }

  bool dir_update_ok = SendToGpu(scene->DirectionalLightsStructuredBuffer, state->device_context.Get());
  if (dir_update_ok) {
    state->device_context->VSSetShaderResources(2, 1, GetAddressOfShaderResourceView(scene->DirectionalLightsStructuredBuffer));
  } else {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error updating directional light buffer");
  }

  // Light data buffer
  auto light_data_buffer = ConstantBuffer::GetCpuBuffer<LightData>(scene->LightDataConstantBuffer);
  light_data_buffer->PointLightCount = GetCurrentSize(scene->PointLightsStructuredBuffer);
  light_data_buffer->SpotLightCount = GetCurrentSize(scene->SpotLightsStructuredBuffer);
  light_data_buffer->DirectionalLightCount = GetCurrentSize(scene->DirectionalLightsStructuredBuffer);

  bool light_data_buffer_update_ok = SendToGpu(scene->LightDataConstantBuffer, state->device_context.Get());
  if (light_data_buffer_update_ok) {
    state->device_context->VSSetConstantBuffers(1, 1, GetAddressOfGpuBuffer(scene->LightDataConstantBuffer));
  } else {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error updating the light data buffer");
  }
}

void Update(Scene* scene, DirectXState* state) {
  // Update the camera
  auto t = static_cast<float>(dxfwGetTime());
  scene->CameraScript.update(t);

  float aspect_ratio = static_cast<float>(state->viewport.Width) / static_cast<float>(state->viewport.Height);

  float frustum_width;
  float frustum_height;
  scene->Lens.UpdateMatrices(aspect_ratio, &frustum_width, &frustum_height);
  scene->Camera.UpdateMatrices(frustum_width, frustum_height);

  UpdateFrameBuffers(scene, state);
}

int main(int /* argc */, char** /* argv */) {
  DxfwGuard dxfw_guard;
  if (!dxfw_guard.IsInitialized()) {
    return -1;
  }

  auto base_path = GetBasePath();

  DirectXState state;
  bool direct3d11_ok = InitializeDirect3d11(&state);
  if (!direct3d11_ok) {
    return -1;
  }

  Scene scene;
  InitializeScene(&state, &scene);
  LoadScene(base_path / "assets/scenes/cube.json", base_path, &state, &scene);

  while (!Dxfw::ShouldWindowClose(state.window.get())) {
    Update(&scene, &state);

    Render(&scene, &state);

    state.swap_chain->Present(0, 0);

    Dxfw::PollOsEvents();
  }

  state.device_context->ClearState();

  return 0;
}
