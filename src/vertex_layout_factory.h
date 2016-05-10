#pragma once

#include <vector>

#include <d3d11.h>

#include "resource_array.h"

/*
bool CreateVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout, ID3DBlob* shader_blob, ID3D11Device* device, ID3D11InputLayout** vertex_layout);
*/

using VertexLayoutHandleType = ResourceArray::HandleType;

ResourceArray::HandleType CreateVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout, ID3DBlob* shader_blob, ID3D11Device* device);

ID3D11InputLayout* GetVertexLayoutFromFactory(ResourceArray::HandleType handle);

