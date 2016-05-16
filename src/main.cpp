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
#include "buffer.h"
#include "mesh_loader.h"
#include "gpu_mesh.h"
#include "vertex_layout_factory.h"
#include "material.h"
#include "drawable.h"
#include "shaders/hlsl_definitions.h"
#include "screen.h"
#include "perspective_lens.h"
#include "trackball_camera.h"

struct DirectXState {
  Microsoft::WRL::ComPtr<ID3D11Device> device;
  Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
};

struct PerFrameConstantBuffer {
  DirectX::XMMATRIX ModelMatrix;
  DirectX::XMMATRIX ViewMatrix;
  DirectX::XMMATRIX ProjectionMatrix;
  DirectX::XMMATRIX NormalMatrix;
  DirectX::XMMATRIX ModelViewMatrix;
  DirectX::XMMATRIX ModelViewProjectionMatrix;
};

struct Scene {
  std::vector<GpuMesh> meshes;
  std::vector<Drawable> drawables;
  Material material;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> vertex_layout;
  D3D11_VIEWPORT viewport;
  PerspectiveLens lens;
  TrackballCamera camera;
};

bool InitializeDeviceAndSwapChain(dxfwWindow* window, DirectXState* state) {
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

  HWND window_handle = Dxfw::GetWindowHandle(window);
  uint32_t width;
  uint32_t height;
  Dxfw::GetWindowSize(window, &width, &height);

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

bool InitializeDirect3d11(dxfwWindow* window, DirectXState* state) {
  // Create device
  bool device_ok = InitializeDeviceAndSwapChain(window, state);
  if (!device_ok) {
    return false;
  }

  bool rt_ok = InitializeRenderTarget(state);
  if (!rt_ok) {
    return false;
  }

  return true;
}

void SetViewportSize(D3D11_VIEWPORT* viewport, unsigned int width, unsigned int height) {
  ZeroMemory(viewport, sizeof(D3D11_VIEWPORT));
  viewport->TopLeftX = 0;
  viewport->TopLeftY = 0;
  viewport->Width = static_cast<float>(width);
  viewport->Height = static_cast<float>(height);
}

bool InitializeScene(const filesystem::path& base_path, dxfwWindow* window, DirectXState* state, Scene* scene) {
  bool vs_ok = LoadVertexShader(base_path / "vs.cso", std::unordered_map<std::string, VertexDataChannel>(), state->device.Get(), &scene->material.VertexShader);
  if (!vs_ok) {
    return false;
  }

  bool ps_ok = LoadPixelShader(base_path / "ps.cso", state->device.Get(), &scene->material.PixelShader);
  if (!ps_ok) {
    return false;
  }

  MeshLoaderOptions options;
  options.IndexBufferFormat = DXGI_FORMAT_R32_UINT;
  bool load_ok = LoadMesh("cube", base_path / "assets/meshes/cube.obj", options, state->device.Get(), &scene->meshes);
  if (!load_ok) {
    return false;
  }

  scene->drawables.reserve(scene->meshes.size());
  for (const auto& mesh : scene->meshes) {
    scene->drawables.emplace_back();
    auto& drawable = scene->drawables.back();
    
    bool drawable_ok = CreateDrawable(mesh, scene->material, state->device.Get(), &drawable);
    if (!drawable_ok) {
      return false;
    }
  }

  uint32_t width;
  uint32_t height;
  Dxfw::GetWindowSize(window, &width, &height);
  
  SetViewportSize(&scene->viewport, width, height);
  state->device_context->RSSetViewports(1, &scene->viewport);

  scene->lens.SetFrustum(1, 3, static_cast<float>(width) / static_cast<float>(height), DirectX::XM_PIDIV2);

  scene->camera.SetRadius(2.0f);
  scene->camera.SetLocation(0, 0, 0);

  Dxfw::RegisterWindowResizeCallback(window, [state, scene](dxfwWindow* /* window */, uint32_t width, uint32_t height){
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
    SetViewportSize(&scene->viewport, width, height);
    state->device_context->RSSetViewports(1, &scene->viewport);
  });

  Dxfw::RegisterMouseButtonCallback(window, [scene](dxfwWindow*, dxfwMouseButton button, dxfwMouseButtonAction action, int16_t x, int16_t y) {
    if (button == DXFW_RIGHT_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_DOWN) {
      scene->camera.SetDesiredState(TrackballCameraOperation::Panning);
      scene->camera.SetEndPoint(GetNormalizedScreenCoordinates(scene->viewport.Width, scene->viewport.Height, x, y));
    } else if (button == DXFW_RIGHT_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_UP) {
      scene->camera.SetDesiredState(TrackballCameraOperation::None);
    } else if (button == DXFW_LEFT_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_DOWN) {
      scene->camera.SetDesiredState(TrackballCameraOperation::Rotating);
      scene->camera.SetEndPoint(GetNormalizedScreenCoordinates(scene->viewport.Width, scene->viewport.Height, x, y));
    } else if (button == DXFW_LEFT_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_UP) {
      scene->camera.SetDesiredState(TrackballCameraOperation::None);
    } else if (button == DXFW_MIDDLE_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_DOWN) {
      scene->camera.SetDesiredState(TrackballCameraOperation::Zooming);
      scene->camera.SetEndPoint(GetNormalizedScreenCoordinates(scene->viewport.Width, scene->viewport.Height, x, y));
    } else if (button == DXFW_MIDDLE_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_UP) {
      scene->camera.SetDesiredState(TrackballCameraOperation::None);
    }
  });

  Dxfw::RegisterMouseWheelCallback(window, [scene](dxfwWindow*, int16_t, int16_t, int16_t delta){
    if (delta > 0) {
      scene->lens.SetZoomFactor(1.1f * scene->lens.GetZoomFactor());
    } else {
      scene->lens.SetZoomFactor(0.9f * scene->lens.GetZoomFactor());
    }
  });

  Dxfw::RegisterMouseMoveCallback(window, [scene](dxfwWindow*, int16_t x, int16_t y){
    scene->camera.SetEndPoint(GetNormalizedScreenCoordinates(scene->viewport.Width, scene->viewport.Height, x, y));
  });

  return true;
}

void Render(Scene* scene, ID3D11Buffer* perFrameConstantBuffer, DirectXState* state) {
  state->device_context->VSSetConstantBuffers(PER_FRAME_CB_INDEX, 1, &perFrameConstantBuffer);

  for (auto& drawable : scene->drawables) {
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

  dxfwWindowUniquePtr window(Dxfw::CreateNewWindow(800, 600, "Hello DirectX"));
  if (!window) {
    return -1;
  }

  DirectXState state;
  bool direct3d11_ok = InitializeDirect3d11(window.get(), &state);
  if (!direct3d11_ok) {
    return -1;
  }

  Scene scene;
  bool scene_ok = InitializeScene(base_path, window.get(), &state, &scene);
  if (!scene_ok) {
    return -1;
  }

  Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;
  bool cb_ok = CrateConstantBuffer<PerFrameConstantBuffer>(nullptr, state.device.Get(), constant_buffer.GetAddressOf());
  if (!cb_ok) {
    return -1;
  }

  PerFrameConstantBuffer perFrameConstaneBuffer;
  DirectX::XMVECTOR axis = { 1, 1, 1, 0 };
  while (!Dxfw::ShouldWindowClose(window.get())) {
    // Update the camera
    auto t = static_cast<float>(dxfwGetTime());
    scene.camera.LookAt(2.0f * DirectX::XMScalarCos(t), 0.0f, 2.0f * DirectX::XMScalarSin(t), 0, 0, 0);

    // Get aspect ratio
    float aspect_ratio = static_cast<float>(scene.viewport.Width) / static_cast<float>(scene.viewport.Height);

    // Update the scene
    float frustum_width;
    float frustum_height;
    scene.lens.UpdateMatrices(aspect_ratio, &frustum_width, &frustum_height);
    scene.camera.UpdateMatrices(frustum_width, frustum_height);

    // Update constant buffers contents
    auto R = DirectX::XMMatrixRotationAxis(axis, DirectX::XM_PIDIV2);
    auto S = DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);
    auto SInv = DirectX::XMMatrixScaling(2.0f, 2.0f, 2.0f);

    perFrameConstaneBuffer.ModelMatrix = S * R;
    perFrameConstaneBuffer.ViewMatrix = scene.camera.GetViewMatrix();
    perFrameConstaneBuffer.ProjectionMatrix = scene.lens.GetProjectionMatrix();
    perFrameConstaneBuffer.NormalMatrix = SInv * R * DirectX::XMMatrixTranspose(scene.camera.GetViewMatrixInverse());
    perFrameConstaneBuffer.ModelViewMatrix = perFrameConstaneBuffer.ModelMatrix * perFrameConstaneBuffer.ViewMatrix;
    perFrameConstaneBuffer.ModelViewProjectionMatrix = perFrameConstaneBuffer.ModelViewMatrix * perFrameConstaneBuffer.ProjectionMatrix;

    // Update constant buffer
    bool update_ok = UpdateConstantBuffer(&perFrameConstaneBuffer, state.device_context.Get(), constant_buffer.Get());
    if (!update_ok) {
      return -1;
    }

    // Clear
    float bgColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    state.device_context->ClearRenderTargetView(state.render_target_view.Get(), bgColor);

    Render(&scene, constant_buffer.Get(), &state);

    state.swap_chain->Present(0, 0);

    Dxfw::PollOsEvents();
  }

  state.device_context->ClearState();

  return 0;
}
