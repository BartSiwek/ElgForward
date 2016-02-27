#include "shader.h"

#include <d3d11.h>
#include <D3Dcompiler.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "filesystem.h"

bool LoadVertexShader(const filesystem::path& path, ID3D11Device* device, VertexShader* vertex_shader,
                      const char* entry_point, const char* target) {
  Microsoft::WRL::ComPtr<ID3DBlob> error_blob;

  HRESULT shader_compilation_result = D3DCompileFromFile(path.c_str(), 
                                                         nullptr, 
                                                         D3D_COMPILE_STANDARD_FILE_INCLUDE, 
                                                         entry_point,
                                                         target, 
                                                         0, 
                                                         0, 
                                                         vertex_shader->Buffer.GetAddressOf(),
                                                         error_blob.GetAddressOf());
  
  if (FAILED(shader_compilation_result)) {
    if (error_blob) {
      DXFW_TRACE(__FILE__, __LINE__, (const char*)error_blob->GetBufferPointer(), false);
    }
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, shader_compilation_result, true);
    return false;
  }

  HRESULT shader_creation_result = device->CreateVertexShader(vertex_shader->Buffer->GetBufferPointer(), 
                                                              vertex_shader->Buffer->GetBufferSize(), 
                                                              nullptr,
                                                              vertex_shader->Shader.GetAddressOf());
  
  if (FAILED(shader_creation_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, shader_creation_result, true);
    return false;
  }

  return true;
}

bool LoadPixelShader(const filesystem::path& path, ID3D11Device* device, PixelShader* pixel_shader,
                     const char* entry_point, const char* target) {
  Microsoft::WRL::ComPtr<ID3DBlob> error_blob;

  HRESULT shader_compilation_result = D3DCompileFromFile(path.c_str(),
                                                         nullptr,
                                                         D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                                         entry_point,
                                                         target,
                                                         0,
                                                         0,
                                                         pixel_shader->Buffer.GetAddressOf(),
                                                         error_blob.GetAddressOf());

  if (FAILED(shader_compilation_result)) {
    if (error_blob) {
      DXFW_TRACE(__FILE__, __LINE__, (const char*)error_blob->GetBufferPointer(), false);
    }
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, shader_compilation_result, true);
    return false;
  }

  HRESULT shader_creation_result = device->CreatePixelShader(pixel_shader->Buffer->GetBufferPointer(),
                                                             pixel_shader->Buffer->GetBufferSize(),
                                                             nullptr,
                                                             pixel_shader->Shader.GetAddressOf());

  if (FAILED(shader_creation_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, shader_creation_result, true);
    return false;
  }

  return true;
}