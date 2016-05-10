#include "vertex_layout_factory.h"

#include <array>

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "shader.h"
#include "resource_array.h"

/*
inline bool CreateVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout, ID3DBlob* shader_blob, ID3D11Device* device, ID3D11InputLayout** vertex_layout) {
  HRESULT create_input_layout_result = device->CreateInputLayout(&input_layout[0],
                                                                 input_layout.size(),
                                                                 shader_blob->GetBufferPointer(),
                                                                 shader_blob->GetBufferSize(),
                                                                 vertex_layout);
  if (FAILED(create_input_layout_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, create_input_layout_result);
    return false;
  }

  return true;
};
*/

ResourceArray g_storage_;

ResourceArray::HandleType CreateVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout, ID3DBlob* shader_blob, ID3D11Device* device) {
  Microsoft::WRL::ComPtr<ID3D11InputLayout> vertex_layout;
  auto create_input_layout_result = device->CreateInputLayout(&input_layout[0],
                                                              input_layout.size(),
                                                              shader_blob->GetBufferPointer(),
                                                              shader_blob->GetBufferSize(),
                                                              vertex_layout.GetAddressOf());

  if (FAILED(create_input_layout_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, create_input_layout_result);
    return {};
  }

  return g_storage_.Add(vertex_layout);
};

ID3D11InputLayout* GetVertexLayoutFromFactory(ResourceArray::HandleType handle) {
  if (g_storage_.IsActive(handle)) {
    return g_storage_.Get(handle).Get();
  }
  return nullptr;
}
