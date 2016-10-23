#pragma once

#include <utility>
#include <cstdint>

#include <d3d11.h>

namespace Rendering {

std::pair<uint32_t, D3D_REGISTER_COMPONENT_TYPE> DxgiFormatToComponentsAndType(DXGI_FORMAT format);

}  // namespace Rendering
