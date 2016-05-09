#pragma once

#include <array>

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "shader.h"
#include "resource_array.h"

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
