#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace filesystem = std::experimental::filesystem;

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <dxgi.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <D3Dcompiler.h>
#include <wrl.h>
#endif

#include <tiny_obj_loader.h>

#include <dxfw/dxfw.h>

#include "interleaved_mesh.h"

class DxfwGuard {
public:
  DxfwGuard() : m_is_initialized_(false) {
    m_is_initialized_ = dxfwInitialize();
  }

  DxfwGuard(const DxfwGuard&) = delete;
  DxfwGuard& operator=(const DxfwGuard&) = delete;
  DxfwGuard(DxfwGuard&&) = delete;
  DxfwGuard& operator=(DxfwGuard&&) = delete;

  ~DxfwGuard() {
    if (m_is_initialized_) {
      dxfwTerminate();
    }
  }

  bool IsInitialized() {
    return m_is_initialized_;
  }

private:
  bool m_is_initialized_;
};

class DxfwWindowDeleter {
 public:
  void operator()(dxfwWindow* window) const {
    dxfwDestroyWindow(window);
  }
};

struct Vertex {
  Vertex() {
  }

  Vertex(float x, float y, float z) : position(x, y, z) {
  }

  DirectX::XMFLOAT3 position;
};

struct DirectXState {
  Microsoft::WRL::ComPtr<ID3D11Device> device;
  Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
};

struct Scene {
  Microsoft::WRL::ComPtr<ID3D11Buffer> triangle_vertex_buffer;
  Microsoft::WRL::ComPtr<ID3D11Buffer> triangle_index_buffer;
  Microsoft::WRL::ComPtr<ID3DBlob> vs_buffer;
  Microsoft::WRL::ComPtr<ID3DBlob> ps_buffer;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> vertex_layout;
};

void ErrorCallback(dxfwError error) {
  DXFW_ERROR_TRACE(__FILE__, __LINE__, error, true);
}

filesystem::path GetBasePath() {
  wchar_t path[MAX_PATH];
  GetModuleFileNameW(nullptr, path, MAX_PATH);
  PathRemoveFileSpecW(path);
  return filesystem::path(path);
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

bool InitializeVertexShader(const filesystem::path& base_path, ID3D11Device* device, ID3DBlob** buffer, ID3D11VertexShader** vs) {
  Microsoft::WRL::ComPtr<ID3DBlob> error_blob;

  filesystem::path full_path = base_path / "assets/shaders/shaders.hlsl";
    
  HRESULT shader_compilation_result = D3DCompileFromFile(full_path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_4_0", 0, 0, buffer, error_blob.GetAddressOf());
  if (FAILED(shader_compilation_result))
  {
    if (error_blob) {
      DXFW_TRACE(__FILE__, __LINE__, (const char*)error_blob->GetBufferPointer(), false);
    }
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, shader_compilation_result, true);
    return false;
  }

  HRESULT shader_creation_result = device->CreateVertexShader((*buffer)->GetBufferPointer(), (*buffer)->GetBufferSize(), NULL, vs);
  if (FAILED(shader_creation_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, shader_creation_result, true);
    return false;
  }

  return true;
}

bool InitializePixelShader(const filesystem::path& base_path, ID3D11Device* device, ID3DBlob** buffer, ID3D11PixelShader** ps) {
  Microsoft::WRL::ComPtr<ID3DBlob> error_blob;

  filesystem::path full_path = base_path / "assets/shaders/shaders.hlsl";

  HRESULT shader_compilation_result = D3DCompileFromFile(full_path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_4_0", 0, 0, buffer, error_blob.GetAddressOf());
  if (FAILED(shader_compilation_result))
  {
    if (error_blob) {
      DXFW_TRACE(__FILE__, __LINE__, (const char*)error_blob->GetBufferPointer(), false);
    }
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, shader_compilation_result, true);
    return false;
  }

  HRESULT shader_creation_result = device->CreatePixelShader((*buffer)->GetBufferPointer(), (*buffer)->GetBufferSize(), NULL, ps);
  if (FAILED(shader_creation_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, shader_creation_result, true);
    return false;
  }

  return true;
}

bool CreateVertexBuffer(const std::vector<Vertex>& data, ID3D11Device* device, ID3D11Buffer** buffer) {
  D3D11_BUFFER_DESC bufferDesc;
  ZeroMemory(&bufferDesc, sizeof(bufferDesc));

  bufferDesc.Usage = D3D11_USAGE_DEFAULT;
  bufferDesc.ByteWidth = data.size() * sizeof(Vertex);
  bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bufferDesc.CPUAccessFlags = 0;
  bufferDesc.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA bufferData;
  ZeroMemory(&bufferData, sizeof(bufferData));
  bufferData.pSysMem = &data[0];

  HRESULT create_buffer_result = device->CreateBuffer(&bufferDesc, &bufferData, buffer);
  
  if (FAILED(create_buffer_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, create_buffer_result, true);
    return false;
  }

  return true;
}

bool CreateIndexBuffer(const std::vector<unsigned int>& data, ID3D11Device* device, ID3D11Buffer** buffer) {
  D3D11_BUFFER_DESC bufferDesc;
  ZeroMemory(&bufferDesc, sizeof(bufferDesc));

  bufferDesc.Usage = D3D11_USAGE_DEFAULT;
  bufferDesc.ByteWidth = data.size() * sizeof(unsigned int);
  bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bufferDesc.CPUAccessFlags = 0;
  bufferDesc.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA bufferData;
  ZeroMemory(&bufferData, sizeof(bufferData));
  bufferData.pSysMem = &data[0];

  HRESULT create_buffer_result = device->CreateBuffer(&bufferDesc, &bufferData, buffer);

  if (FAILED(create_buffer_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, create_buffer_result, true);
    return false;
  }

  return true;
}

bool CreateInputLayout(ID3D11Device* device, ID3DBlob* vs_buffer, ID3D11InputLayout** vertex_layout) {
  D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
  };
  UINT layout_elements_count = 1;

  HRESULT create_input_layout_result = device->CreateInputLayout(layout, layout_elements_count, vs_buffer->GetBufferPointer(), vs_buffer->GetBufferSize(), vertex_layout);
  if (FAILED(create_input_layout_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, create_input_layout_result, true);
    return false;
  }

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

bool LoadModel(const filesystem::path& base_path, ID3D11Device* device, ID3D11Buffer** triangle_vertex_buffer, ID3D11Buffer** triangle_index_buffer) {
  auto mesh_path = base_path / "assets/meshes/cube.obj";
  auto mesh_path_string = mesh_path.string();

  auto materials_base_path = base_path / "assets/materials/";
  auto materials_base_path_string = materials_base_path.string();

  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;
  bool ret = tinyobj::LoadObj(shapes, materials, err, mesh_path_string.c_str(), materials_base_path_string.c_str());

  if (!err.empty()) { // `err` may contain warning message.
    DXFW_TRACE(__FILE__, __LINE__, err.c_str(), true);
  }

  if (!ret) {
    return false;
  }

  const auto& shape = shapes[0];
  size_t vertex_count = shape.mesh.positions.size();

  std::vector<Vertex> vertices;
  vertices.reserve(vertex_count);
  for (size_t v = 0; v < shapes[0].mesh.positions.size(); v += 3) {
    vertices.emplace_back(shape.mesh.positions[v], shape.mesh.positions[v + 1], shape.mesh.positions[v + 2]);
  }
  
  CreateVertexBuffer(vertices, device, triangle_vertex_buffer);
  CreateIndexBuffer(shape.mesh.indices, device, triangle_index_buffer);


  std::cout << "# of shapes    : " << shapes.size() << std::endl;
  std::cout << "# of materials : " << materials.size() << std::endl;

  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", i, shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %ld\n", i, shapes[i].mesh.indices.size());
    printf("Size of shape[%ld].material_ids: %ld\n", i, shapes[i].mesh.material_ids.size());
    assert((shapes[i].mesh.indices.size() % 3) == 0);
    for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
      unsigned int a = shapes[i].mesh.indices[3 * f + 0];
      unsigned int b = shapes[i].mesh.indices[3 * f + 1];
      unsigned int c = shapes[i].mesh.indices[3 * f + 2];

      printf("  idx[%ld] = %d, %d, %d. mat_id = %d\n", f, a, b, c, shapes[i].mesh.material_ids[f]);
      printf("  (%f, %f, %f) -> (%f, %f, %f) -> (%f, %f, %f)\n",
        shapes[i].mesh.positions[3 * a + 0], shapes[i].mesh.positions[3 * a + 1], shapes[i].mesh.positions[3 * a + 2],
        shapes[i].mesh.positions[3 * b + 0], shapes[i].mesh.positions[3 * b + 1], shapes[i].mesh.positions[3 * b + 2],
        shapes[i].mesh.positions[3 * c + 0], shapes[i].mesh.positions[3 * c + 1], shapes[i].mesh.positions[3 * c + 2]);
    }

    printf("shape[%ld].vertices: %ld\n", i, shapes[i].mesh.positions.size());
    assert((shapes[i].mesh.positions.size() % 3) == 0);
    for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
      printf("  v[%ld] = (%f, %f, %f)\n", v,
        shapes[i].mesh.positions[3 * v + 0],
        shapes[i].mesh.positions[3 * v + 1],
        shapes[i].mesh.positions[3 * v + 2]);
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", i, materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n", materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
    printf("  material.Kd = (%f, %f ,%f)\n", materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
    printf("  material.Ks = (%f, %f ,%f)\n", materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
    printf("  material.Tr = (%f, %f ,%f)\n", materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]);
    printf("  material.Ke = (%f, %f ,%f)\n", materials[i].emission[0], materials[i].emission[1], materials[i].emission[2]);
    printf("  material.Ns = %f\n", materials[i].shininess);
    printf("  material.Ni = %f\n", materials[i].ior);
    printf("  material.dissolve = %f\n", materials[i].dissolve);
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n", materials[i].specular_highlight_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(materials[i].unknown_parameter.end());
    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }

  return true;
}

bool InitializeScene(const filesystem::path& base_path, dxfwWindow* window, DirectXState* state, Scene* scene) {
  bool vs_ok = InitializeVertexShader(base_path, state->device.Get(), scene->vs_buffer.GetAddressOf(), scene->vs.GetAddressOf());
  if (!vs_ok) {
    return false;
  }

  bool ps_ok = InitializePixelShader(base_path, state->device.Get(), scene->ps_buffer.GetAddressOf(), scene->ps.GetAddressOf());
  if (!ps_ok) {
    return false;
  }

  state->device_context->VSSetShader(scene->vs.Get(), 0, 0);
  state->device_context->PSSetShader(scene->ps.Get(), 0, 0);

  bool model_ok = LoadModel(base_path, state->device.Get(), scene->triangle_vertex_buffer.GetAddressOf(), scene->triangle_index_buffer.GetAddressOf());
  if (!model_ok) {
    return false;
  }

  UINT stride = sizeof(Vertex);
  UINT offset = 0;
  state->device_context->IASetVertexBuffers(0, 1, scene->triangle_vertex_buffer.GetAddressOf(), &stride, &offset);
  state->device_context->IASetIndexBuffer(scene->triangle_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

  bool il_ok = CreateInputLayout(state->device.Get(), scene->vs_buffer.Get(), scene->vertex_layout.GetAddressOf());
  if (!il_ok) {
    return false;
  }

  state->device_context->IASetInputLayout(scene->vertex_layout.Get());
  state->device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  D3D11_VIEWPORT viewport;
  CreateViewport(window, &viewport);
  state->device_context->RSSetViewports(1, &viewport);

  return true;
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

  while (!dxfwShouldWindowClose(window.get())) {
    // Clear
    float bgColor[4] = { (0.0f, 0.0f, 0.0f, 0.0f) };
    state.device_context->ClearRenderTargetView(state.render_target_view.Get(), bgColor);

    // Render
    state.device_context->DrawIndexed(36, 0, 0);

    // Swap buffers
    state.swap_chain->Present(0, 0);

    dxfwPollOsEvents();
  }

  state.device_context->ClearState();

  return 0;
}
