#include "shader.h"

#include <d3d11.h>
#include <D3Dcompiler.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include <sstream>

#include "filesystem.h"

bool ReflectShader(ID3DBlob* blob) {
  Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflector;
  HRESULT reflector_creation_result = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)reflector.GetAddressOf());
  if (FAILED(reflector_creation_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, reflector_creation_result, true);
    return false;
  }

  D3D11_SHADER_DESC shader_desc;
  HRESULT get_desc_result = reflector->GetDesc(&shader_desc);
  if (FAILED(get_desc_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, get_desc_result, true);
    return false;
  }

  D3D11_SIGNATURE_PARAMETER_DESC param_desc;
  for (uint32_t i = 0; i < shader_desc.InputParameters; ++i) {
    reflector->GetInputParameterDesc(i, &param_desc);

    std::stringstream ss;
    ss << "Param " << i << ": " << param_desc.SemanticName << " " << param_desc.SemanticIndex << " " << param_desc.Mask << " " << param_desc.ComponentType;

    DXFW_TRACE(__FILE__, __LINE__, ss.str().c_str(), false);
  }

  return true;
}

bool LoadVertexShader(const filesystem::path& path, ID3D11Device* device, VertexShader* vertex_shader) {
  HRESULT load_result = D3DReadFileToBlob(path.c_str(), vertex_shader->Buffer.GetAddressOf());
  if (FAILED(load_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, load_result, true);
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

  bool relfection_ok = ReflectShader(vertex_shader->Buffer.Get());
  if (!relfection_ok) {
    return false;
  }

  return true;
}

bool LoadPixelShader(const filesystem::path& path, ID3D11Device* device, PixelShader* pixel_shader) {
  HRESULT load_result = D3DReadFileToBlob(path.c_str(), pixel_shader->Buffer.GetAddressOf());
  if (FAILED(load_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, load_result, true);
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
