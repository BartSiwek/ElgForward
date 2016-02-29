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

#include <dxfw/dxfw.h>

#include "filesystem.h"
#include "dxfw_helpers.h"
#include "mesh.h"
#include "mesh_loader.h"
#include "gpu_mesh.h"
#include "gpu_mesh_factory.h"
#include "vertex_layout_factory.h"
#include "material.h"
#include "shaders/hlsl_definitions.h"

struct DirectXState {
  Microsoft::WRL::ComPtr<ID3D11Device> device;
  Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
};

struct PerFrameConstantBuffer {
  DirectX::XMMATRIX ModelViewMatrix;
};

struct Scene {
  std::vector<GpuMesh> meshes;
  Material material;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> vertex_layout;
};

void ErrorCallback(dxfwError error) {
  DXFW_ERROR_TRACE(__FILE__, __LINE__, error, true);
}

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

  HWND window_handle = dxfwGetHandle(window);
  uint32_t width;
  uint32_t height;
  dxfwGetWindowSize(window, &width, &height);

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
    hr = D3D11CreateDeviceAndSwapChain(NULL, driver_type, NULL, create_device_flags, feature_levels,
      num_feature_levels, D3D11_SDK_VERSION, &swap_chain_desc,
      state->swap_chain.GetAddressOf(), state->device.GetAddressOf(),
      &result_feature_level, state->device_context.GetAddressOf());

    if (SUCCEEDED(hr)) {
      break;
    }

    if (FAILED(hr)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, hr, false);
    }
  }

  if (FAILED(hr)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, hr, true);
    return false;
  }

  return true;
}

bool InitializeDirect3d11(dxfwWindow* window, DirectXState* state) {
  // Create device
  bool device_ok = InitializeDeviceAndSwapChain(window, state);
  if (!device_ok) {
    return false;
  }

  // Create our BackBuffer
  ID3D11Texture2D* back_buffer;
  auto back_buffer_result = state->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));
  if (FAILED(back_buffer_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, back_buffer_result, true);
    return false;
  }

  // Create our Render Target
  auto rtv_result = state->device->CreateRenderTargetView(back_buffer, NULL, state->render_target_view.GetAddressOf());
  if (FAILED(rtv_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, rtv_result, true);
    return false;
  }

  // Back buffer is held by RTV, so we can release it here
  back_buffer->Release();

  // Set our Render Target
  state->device_context->OMSetRenderTargets(1, state->render_target_view.GetAddressOf(), NULL);

  return true;
}

void CreateViewport(dxfwWindow* window, D3D11_VIEWPORT* viewport) {
  ZeroMemory(viewport, sizeof(D3D11_VIEWPORT));

  uint32_t width;
  uint32_t height;
  dxfwGetWindowSize(window, &width, &height);

  viewport->TopLeftX = 0;
  viewport->TopLeftY = 0;
  viewport->Width = static_cast<float>(width);
  viewport->Height = static_cast<float>(height);
}

template<typename BufferType>
bool CrateConstantBuffer(BufferType* initial, DirectXState* state, ID3D11Buffer** constant_buffer) {
  D3D11_BUFFER_DESC desc;
  desc.ByteWidth = sizeof(BufferType);
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  desc.MiscFlags = 0;
  desc.StructureByteStride = 0;

  HRESULT cb_result;
  if (initial != nullptr) {
    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = initial;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;

    cb_result = state->device->CreateBuffer(&desc, &data, constant_buffer);
  } else {
    cb_result = state->device->CreateBuffer(&desc, nullptr, constant_buffer);
  }

  if (FAILED(cb_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, cb_result, true);
    return false;
  }

  return true;
}

template<typename BufferType>
bool UpdateConstantBuffer(BufferType* data, DirectXState* state, ID3D11Buffer* constant_buffer) {
  D3D11_MAPPED_SUBRESOURCE mapped_subresource;

  auto map_result = state->device_context->Map(constant_buffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
  if (FAILED(map_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, map_result, true);
    return false;
  }

  memcpy(mapped_subresource.pData, data, sizeof(BufferType));

  state->device_context->Unmap(constant_buffer, 0);

  return true;
}

bool InitializeScene(const filesystem::path& base_path, dxfwWindow* window, DirectXState* state, Scene* scene) {
  bool vs_ok = LoadVertexShader(base_path / "vs.cso", state->device.Get(), &scene->material.VertexShader);
  if (!vs_ok) {
    return false;
  }

  bool ps_ok = LoadPixelShader(base_path / "ps.cso", state->device.Get(), &scene->material.PixelShader);
  if (!ps_ok) {
    return false;
  }

  std::vector<Mesh> meshes;
  bool load_ok = LoadMesh(base_path / "assets/meshes/cube.obj", &meshes);
  if (!load_ok) {
    return false;
  }

  scene->meshes.reserve(meshes.size());
  for (const auto& mesh : meshes) {
    scene->meshes.emplace_back();
    auto& gpu_mesh = scene->meshes.back();
    bool gpu_mesh_ok = GpuMeshFactory::CreateGpuMesh(mesh, state->device.Get(), &gpu_mesh);
    if (!gpu_mesh_ok) {
      return false;
    }
  }
  bool il_ok = VertexLayoutFactory::CreateVertexLayout(state->device.Get(), &scene->material, scene->vertex_layout.GetAddressOf());
  if (!il_ok) {
    return false;
  }

  D3D11_VIEWPORT viewport;
  CreateViewport(window, &viewport);
  state->device_context->RSSetViewports(1, &viewport);

  return true;
}

void Render(const Scene& scene, ID3D11Buffer* perFrameConstantBuffer, DirectXState* state) {
  state->device_context->VSSetShader(scene.material.VertexShader.Shader.Get(), 0, 0);
  state->device_context->PSSetShader(scene.material.PixelShader.Shader.Get(), 0, 0);

  state->device_context->VSSetConstantBuffers(PER_FRAME_CB_INDEX, 1, &perFrameConstantBuffer);

  for (const auto& mesh : scene.meshes) {
    std::vector<uint32_t> offsets(GpuMesh::VertexBufferCount, 0);
    state->device_context->IASetVertexBuffers(0, GpuMesh::VertexBufferCount, &mesh.VertexBuffers[0], &mesh.VertexBufferStrides[0], &offsets[0]);

    state->device_context->IASetIndexBuffer(mesh.IndexBuffer.Get(), GpuMeshFactory::IndexBufferFormat, 0);
    state->device_context->IASetPrimitiveTopology(GpuMesh::PrimitiveTopology);

    state->device_context->IASetInputLayout(scene.vertex_layout.Get());

    state->device_context->DrawIndexed(mesh.IndexCount, 0, 0);
  }
}

int main(int /* argc */, char** /* argv */) {
  DxfwGuard dxfw_guard;
  if (!dxfw_guard.IsInitialized()) {
    return -1;
  }

  auto base_path = GetBasePath();

  dxfwSetErrorCallback(ErrorCallback);

  std::unique_ptr<dxfwWindow, DxfwWindowDeleter> window(dxfwCreateWindow(600, 600, "Hello DirectX"));
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
  bool cb_ok = CrateConstantBuffer<PerFrameConstantBuffer>(nullptr, &state, constant_buffer.GetAddressOf());
  if (!cb_ok) {
    return -1;
  }

  PerFrameConstantBuffer perFrameConstaneBuffer;
  DirectX::XMVECTOR axis = { 1, 1, 1, 0 };
  while (!dxfwShouldWindowClose(window.get())) {
    // Update constant buffers contents
    float t = (float)fmod(dxfwGetTime(), 2.0);
    perFrameConstaneBuffer.ModelViewMatrix = DirectX::XMMatrixRotationAxis(axis, t * DirectX::XM_PI);

    // Update constant buffer
    bool update_ok = UpdateConstantBuffer(&perFrameConstaneBuffer, &state, constant_buffer.Get());
    if (!update_ok) {
      return -1;
    }

    // Clear
    float bgColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    state.device_context->ClearRenderTargetView(state.render_target_view.Get(), bgColor);

    Render(scene, constant_buffer.Get(), &state);

    state.swap_chain->Present(0, 0);

    dxfwPollOsEvents();
  }

  state.device_context->ClearState();

  return 0;
}
