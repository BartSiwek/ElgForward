#pragma once

#include <vector>

#include <d3d11.h>
#include <wrl.h>

#include "core/handle.h"

namespace Rendering {
namespace VertexLayout {

struct VertexLayoutTag {};

using Handle = Core::Handle<8, 24, VertexLayoutTag>;

Handle Create(const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_layout, ID3DBlob* shader_blob, ID3D11Device* device);

Microsoft::WRL::ComPtr<ID3D11InputLayout> Retreive(Handle handle);

}  // namespace VertexLayout
}  // namespace Rendering
