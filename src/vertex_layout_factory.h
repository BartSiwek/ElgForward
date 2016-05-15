#pragma once

#include <vector>

#include <d3d11.h>

#include "handle.h"

struct VertexLayoutTag {};

using VertexLayoutHandle = Handle<8, 24, VertexLayoutTag>;

VertexLayoutHandle CreateVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout, ID3DBlob* shader_blob, ID3D11Device* device);

ID3D11InputLayout* GetVertexLayoutFromFactory(VertexLayoutHandle handle);

