#pragma once

#include <array>

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "gpu_mesh_factory.h"
#include "material.h"

struct VertexLayoutFactory {
  static bool CreateVertexLayout(ID3D11Device* device, Material* material, ID3D11InputLayout** vertex_layout) {
    HRESULT create_input_layout_result = device->CreateInputLayout(&GpuMeshFactory::InputLayout[0],
                                                                   GpuMeshFactory::InputLayoutElementCount,
                                                                   material->VertexShader.Buffer->GetBufferPointer(),
                                                                   material->VertexShader.Buffer->GetBufferSize(),
                                                                   vertex_layout);
    if (FAILED(create_input_layout_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, create_input_layout_result, true);
      return false;
    }

    return true;
  }
};
