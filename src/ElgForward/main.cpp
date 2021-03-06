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

#include "core/assert.h"
#include "core/filesystem.h"
#include "dxfw/dxfw_wrapper.h"
#include "dxfw/dxfw_helpers.h"
#include "rendering/constant_buffer.h"
#include "rendering/lens/perspective_lens.h"
#include "rendering/cameras/trackball_camera.h"
#include "rendering/lights/directional_light.h"
#include "rendering/lights/point_light.h"
#include "rendering/lights/spot_light.h"
#include "rendering/materials/basic.h"
#include "rendering/mesh.h"
#include "rendering/vertex_layout.h"
#include "rendering/drawable.h"
#include "rendering/screen.h"
#include "shaders/registers.h"
#include "loaders/scene_loader.h"
#include "directx_state.h"
#include "scene.h"

using namespace Rendering;

void InitializeDeviceAndSwapChain(DirectXState* state) {
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

  ASSERT_HRESULT(hr);

  auto debug_result = state->device.As(&state->debug);
  ASSERT_HRESULT(debug_result);
}

bool InitializeRenderTarget(DirectXState* state, uint32_t width, uint32_t height) {
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

  // Depth/stencil buffer settings
  D3D11_TEXTURE2D_DESC depth_stencil_desc;
  ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));

  depth_stencil_desc.Width = width;
  depth_stencil_desc.Height = height;
  depth_stencil_desc.MipLevels = 1;
  depth_stencil_desc.ArraySize = 1;
  depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_stencil_desc.SampleDesc.Count = 1;
  depth_stencil_desc.SampleDesc.Quality = 0;
  depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
  depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depth_stencil_desc.CPUAccessFlags = 0;
  depth_stencil_desc.MiscFlags = 0;

  // Create the depth/stencil buffer
  ID3D11Texture2D* depth_stencil_buffer;
  auto depth_stencil_result = state->device->CreateTexture2D(&depth_stencil_desc, nullptr, &depth_stencil_buffer);
  if (FAILED(depth_stencil_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, depth_stencil_result);
    return false;
  }

  // Depth/stencil view settings
  D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
  ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));

  depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depth_stencil_view_desc.Texture2D.MipSlice = 0;

  // Create the depth/stencil view
  auto depth_stencil_view_result = state->device->CreateDepthStencilView(depth_stencil_buffer, &depth_stencil_view_desc, state->depth_stencil_view.GetAddressOf());
  if (FAILED(depth_stencil_view_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, depth_stencil_view_result);
    return false;
  }

  // The back buffer is held by the depth/stencil view now, release it
  depth_stencil_buffer->Release();

  // Set our Render Target
  state->device_context->OMSetRenderTargets(1, state->render_target_view.GetAddressOf(), state->depth_stencil_view.Get());

  return true;
}

bool InitalizeDepthStencilState(DirectXState* state) {
  // Depth/stencil state settings
  D3D11_DEPTH_STENCIL_DESC desc;
  ZeroMemory(&desc, sizeof(desc));

  desc.DepthEnable = true;
  desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  desc.DepthFunc = D3D11_COMPARISON_LESS;

  desc.StencilEnable = false;
  desc.StencilReadMask = 0xFF;
  desc.StencilWriteMask = 0xFF;

  // Create depth/stencil state
  auto depth_stencil_state_result = state->device->CreateDepthStencilState(&desc, state->depth_stencil_state.GetAddressOf());
  if (FAILED(depth_stencil_state_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, depth_stencil_state_result);
    return false;
  }

  // Set the state
  state->device_context->OMSetDepthStencilState(state->depth_stencil_state.Get(), 1);

  return true;
}

bool InitializeSamplers(DirectXState* state) {
  // Linear sampler
  D3D11_SAMPLER_DESC linear_sampler_desc;
  ZeroMemory(&linear_sampler_desc, sizeof(linear_sampler_desc));

  linear_sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  linear_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  linear_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  linear_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  linear_sampler_desc.MipLODBias = 0.0;
  linear_sampler_desc.MaxAnisotropy = 4;
  linear_sampler_desc.ComparisonFunc = D3D11_COMPARISON_LESS;
  linear_sampler_desc.MinLOD = 0.0f;
  linear_sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

  auto linear_sampler_result = state->device->CreateSamplerState(&linear_sampler_desc, state->linear_sampler.GetAddressOf());
  if (FAILED(linear_sampler_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, linear_sampler_result);
    return false;
  }

  state->device_context->VSSetSamplers(LINEAR_SAMPLER_REGISTER, 1, state->linear_sampler.GetAddressOf());
  state->device_context->PSSetSamplers(LINEAR_SAMPLER_REGISTER, 1, state->linear_sampler.GetAddressOf());

  return true;
}

bool InitializeDirect3d11(DirectXState* state) {
  // Crate window
  const uint32_t DefaultWidth = 800;
  const uint32_t DefaultHeight = 600;

  state->window.reset(Dxfw::CreateNewWindow(DefaultWidth, DefaultHeight, "Hello DirectX"));
  if (!state->window) {
    return false;
  }

  // Create device
  InitializeDeviceAndSwapChain(state);

  // Create RT
  bool rt_ok = InitializeRenderTarget(state, DefaultWidth, DefaultHeight);
  if (!rt_ok) {
    return false;
  }

  // Create the depth/stencil state
  bool depth_stencil_ok = InitalizeDepthStencilState(state);
  if (!depth_stencil_ok) {
    return false;
  }

  // Create samplers
  bool samplers_ok = InitializeSamplers(state);
  if (!samplers_ok) {
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

    bool rt_ok = InitializeRenderTarget(state, width, height);
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
  scene->PerFrameConstantBuffer = ConstantBuffer::Create<PerFrame>("PerFrameConstants", nullptr, state->device.Get());
  if (!scene->PerFrameConstantBuffer.IsValid()) {
    return false;
  }

  scene->PerCameraConstantBuffer = ConstantBuffer::Create<PerCamera>("PerCameraConstants", nullptr, state->device.Get());
  if (!scene->PerCameraConstantBuffer.IsValid()) {
    return false;
  }

  scene->DirectionalLightsStructuredBuffer = StructuredBuffer::Create<Rendering::Lights::DirectionalLight>("DirectionalLights", 1000, nullptr, 0, state->device.Get());
  if (!scene->DirectionalLightsStructuredBuffer.IsValid()) {
    return false;
  }

  scene->SpotLightsStructuredBuffer = StructuredBuffer::Create<Rendering::Lights::SpotLight>("SpotLights", 1000, nullptr, 0, state->device.Get());
  if (!scene->SpotLightsStructuredBuffer.IsValid()) {
    return false;
  }

  scene->PointLightsStructuredBuffer = StructuredBuffer::Create<Rendering::Lights::PointLight>("PointLights", 1000, nullptr, 0, state->device.Get());
  if (!scene->PointLightsStructuredBuffer.IsValid()) {
    return false;
  }

  return true;
}

void SetConstantBuffers(const Drawable& drawable, Scene* scene, DirectXState* state) {
  ID3D11Buffer* constant_buffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = { nullptr };
  constant_buffers[PER_FRAME_CONSTANT_BUFFER_REGISTER] = ConstantBuffer::GetGpuBuffer(scene->PerFrameConstantBuffer).Get();
  constant_buffers[PER_CAMERA_CONSTANT_BUFFER_REGISTER] = ConstantBuffer::GetGpuBuffer(scene->PerCameraConstantBuffer).Get();
  constant_buffers[PER_OBJECT_CONSTANT_BUFFER_REGISTER] = drawable.GetTransformConstantBuffer();
  constant_buffers[PER_MATERIAL_CONSTANT_BUFFER_REGISTER] = drawable.GetMaterialConstantBuffer();
  state->device_context->VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, constant_buffers);
  state->device_context->PSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, constant_buffers);

  bool send_transforms_ok = drawable.SendTransformConstantBufferToGpu(state->device_context.Get());
  if (!send_transforms_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error sending the transform constant buffer data to GPU");
  }

  bool send_material_ok = drawable.SendMaterialConstantBufferToGpu(state->device_context.Get());
  if (!send_material_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error sending the transform constant buffer data to GPU");
  }
}

void SetShaderResources(const Drawable& drawable, Scene* scene, DirectXState* state) {
  // Vertex shader
  Core::ComArray<ID3D11ShaderResourceView, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> vs_shader_resources = {};
  vs_shader_resources.Set(POINT_LIGHT_BUFFER_REGISTER, GetShaderResourceView(scene->PointLightsStructuredBuffer).Get());
  vs_shader_resources.Set(SPOT_LIGHT_BUFFER_REGISTER, GetShaderResourceView(scene->SpotLightsStructuredBuffer).Get());
  vs_shader_resources.Set(DIRECTIONAL_LIGHT_BUFFER_REGISTER, GetShaderResourceView(scene->DirectionalLightsStructuredBuffer).Get());
  
  drawable.BuildVertexShaderResourceView(&vs_shader_resources);
  state->device_context->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &vs_shader_resources.Get(0));

  // Pixel shader
  Core::ComArray<ID3D11ShaderResourceView, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> ps_shader_resources = {};
  ps_shader_resources.Set(POINT_LIGHT_BUFFER_REGISTER, GetShaderResourceView(scene->PointLightsStructuredBuffer).Get());
  ps_shader_resources.Set(SPOT_LIGHT_BUFFER_REGISTER, GetShaderResourceView(scene->SpotLightsStructuredBuffer).Get());
  ps_shader_resources.Set(DIRECTIONAL_LIGHT_BUFFER_REGISTER, GetShaderResourceView(scene->DirectionalLightsStructuredBuffer).Get());

  drawable.BuildPixelShaderResourceView(&ps_shader_resources);
  state->device_context->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, &ps_shader_resources.Get(0));
}

void Render(Scene* scene, DirectXState* state) {
  float bgColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  state->device_context->ClearRenderTargetView(state->render_target_view.Get(), bgColor);
  
  state->device_context->ClearDepthStencilView(state->depth_stencil_view.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

  for (auto& drawable : scene->Drawables) {
    state->device_context->VSSetShader(drawable.GetVertexShader(), 0, 0);
    state->device_context->PSSetShader(drawable.GetPixelShader(), 0, 0);

    SetConstantBuffers(drawable, scene, state);
    SetShaderResources(drawable, scene, state);

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
  for(auto& point_light : scene->PointLightsStructuredBuffer) {
    point_light.Update(scene->Camera.GetViewMatrix());
  }
  
  bool point_update_ok = SendToGpu(scene->PointLightsStructuredBuffer, state->device_context.Get());
  if (!point_update_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error updating point light buffer", "");
  }

  // Spot lights
  for (auto& spot_light : scene->SpotLightsStructuredBuffer) {
    spot_light.Update(scene->Camera.GetViewMatrix());
  }

  bool spot_update_ok = SendToGpu(scene->SpotLightsStructuredBuffer, state->device_context.Get());
  if (!spot_update_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error updating spot light buffer", "");
  }

  // Directional lights
  for (auto& directional_light : scene->DirectionalLightsStructuredBuffer) {
    directional_light.Update(scene->Camera.GetViewMatrix());
  }

  bool dir_update_ok = SendToGpu(scene->DirectionalLightsStructuredBuffer, state->device_context.Get());
  if (!dir_update_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error updating directional light buffer", "");
  }

  // Light data buffer
  auto buffer = ConstantBuffer::GetCpuBuffer(scene->PerFrameConstantBuffer);
  buffer->PointLightCount = static_cast<int>(GetCurrentSize(scene->PointLightsStructuredBuffer));
  buffer->SpotLightCount = static_cast<int>(GetCurrentSize(scene->SpotLightsStructuredBuffer));
  buffer->DirectionalLightCount = static_cast<int>(GetCurrentSize(scene->DirectionalLightsStructuredBuffer));

  bool update_ok = SendToGpu(scene->PerFrameConstantBuffer, state->device_context.Get());
  if (!update_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error updating per frame constant buffer", "");
  }
}

void UpdateCameraBuffers(Scene* scene, DirectXState* state) {
  auto buffer = ConstantBuffer::GetCpuBuffer(scene->PerCameraConstantBuffer);
  buffer->ViewMatrix = scene->Camera.GetViewMatrix();
  buffer->ViewMatrixInverseTranspose = scene->Camera.GetViewMatrixInverseTranspose();
  buffer->ProjectionMatrix = scene->Lens.GetProjectionMatrix();

  bool update_ok = SendToGpu(scene->PerCameraConstantBuffer, state->device_context.Get());
  if (!update_ok) {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error updating per frame constant buffer", "");
  }
}

void UpdateDrawableBuffers(Drawable* /* drawable */, Scene* /* scene */, DirectXState* /* state */) {
  // Put update here
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
  UpdateCameraBuffers(scene, state);

  for (auto& drawable : scene->Drawables) {
    UpdateDrawableBuffers(&drawable, scene, state);
  }
}

int main(int /* argc */, char** /* argv */) {
  Dxfw::DxfwGuard dxfw_guard;
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
  Loaders::LoadScene(base_path / "assets/scenes/cube.json", base_path, &state, &scene);

  while (!Dxfw::ShouldWindowClose(state.window.get())) {
    Update(&scene, &state);

    Render(&scene, &state);

    state.swap_chain->Present(0, 0);

    Dxfw::PollOsEvents();
  }

  state.device_context->ClearState();

  return 0;
}
