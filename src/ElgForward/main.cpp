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

struct PerFrameConstantBuffer {
  DirectX::XMMATRIX ModelMatrix;
  DirectX::XMMATRIX ViewMatrix;
  DirectX::XMMATRIX ProjectionMatrix;
  DirectX::XMMATRIX NormalMatrix;
  DirectX::XMMATRIX ModelViewMatrix;
  DirectX::XMMATRIX ModelViewProjectionMatrix;
};

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

void UpdateConstantBuffers(const Drawable& drawable, Scene* scene, PerFrameConstantBuffer* cpuPerFrameConstantBuffer, ID3D11Buffer* gpuPerFrameConstantBuffer, DirectXState* state) {
  // Update constant buffers contents
  cpuPerFrameConstantBuffer->ModelMatrix = drawable.GetModelMatrix();
  cpuPerFrameConstantBuffer->ViewMatrix = scene->camera.GetViewMatrix();
  cpuPerFrameConstantBuffer->ProjectionMatrix = scene->lens.GetProjectionMatrix();
  cpuPerFrameConstantBuffer->NormalMatrix = drawable.GetModelMatrixInverseTranspose() * scene->camera.GetViewMatrixInverseTranspose();
  cpuPerFrameConstantBuffer->ModelViewMatrix = cpuPerFrameConstantBuffer->ModelMatrix * cpuPerFrameConstantBuffer->ViewMatrix;
  cpuPerFrameConstantBuffer->ModelViewProjectionMatrix = cpuPerFrameConstantBuffer->ModelViewMatrix * cpuPerFrameConstantBuffer->ProjectionMatrix;

  // Update constant buffer
  bool update_ok = UpdateConstantBuffer(cpuPerFrameConstantBuffer, state->device_context.Get(), gpuPerFrameConstantBuffer);
  if (update_ok) {
    state->device_context->VSSetConstantBuffers(0, 1, &gpuPerFrameConstantBuffer); 
  } else {
    DXFW_TRACE(__FILE__, __LINE__, false, "Error updating per frame constant buffer");
  }
}

void Render(Scene* scene, PerFrameConstantBuffer* cpuPerFrameConstantBuffer, ID3D11Buffer* gpuPerFrameConstantBuffer, DirectXState* state) {
  for (auto& drawable : scene->drawables) {
    UpdateConstantBuffers(drawable, scene, cpuPerFrameConstantBuffer, gpuPerFrameConstantBuffer, state);

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
  LoadScene(base_path / "assets/scenes/cube.json", base_path, &state, &scene);

  // Constant buffers - fold into one struct
  Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;
  bool cb_ok = CrateConstantBuffer<PerFrameConstantBuffer>(nullptr, state.device.Get(), constant_buffer.GetAddressOf());
  if (!cb_ok) {
    return -1;
  }

  PerFrameConstantBuffer perFrameConstaneBuffer;
  // ---------------------

  while (!Dxfw::ShouldWindowClose(state.window.get())) {
    // Update the camera
    auto t = static_cast<float>(dxfwGetTime());
    scene.camera_script.update(t);

    // Get aspect ratio
    float aspect_ratio = static_cast<float>(state.viewport.Width) / static_cast<float>(state.viewport.Height);

    // Update the scene
    float frustum_width;
    float frustum_height;
    scene.lens.UpdateMatrices(aspect_ratio, &frustum_width, &frustum_height);
    scene.camera.UpdateMatrices(frustum_width, frustum_height);

    // Clear
    float bgColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    state.device_context->ClearRenderTargetView(state.render_target_view.Get(), bgColor);

    Render(&scene, &perFrameConstaneBuffer, constant_buffer.Get(), &state);

    state.swap_chain->Present(0, 0);

    Dxfw::PollOsEvents();
  }

  state.device_context->ClearState();

  return 0;
}
