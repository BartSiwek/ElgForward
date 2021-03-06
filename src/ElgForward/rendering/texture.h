#pragma once

#include <vector>

#include <d3d11.h>
#include <wrl.h>

#include "core/filesystem.h"
#include "core/handle.h"

namespace Rendering {
namespace Texture {

struct TextureTag {};

using Handle = Core::Handle<8, 24, TextureTag>;

enum class Type {
  UNKNOWN,
  DIM_1,
  DIM_1_ARRAY,
  DIM_2,
  DIM_2_ARRAY,
  DIM_2_MULTI,
  DIM_2_MULTI_ARRAY,
  DIM_3,
  CUBE,
  CUBE_ARRAY
};

struct ImageData {
  ImageData(uint32_t width, uint32_t height, uint8_t components, DXGI_FORMAT format, const unsigned char* data)
      : Width(width), Height(height), Components(components), Format(format), Data(data) {
  }

  uint32_t Width;
  uint32_t Height;
  uint8_t Components;
  DXGI_FORMAT Format;
  const unsigned char* Data;
};

Handle Create(size_t name_hash, const std::vector<ImageData>& data, ID3D11Device* device);

Handle Create(const std::string& name, const std::vector<ImageData>& data, ID3D11Device* device);

DXGI_FORMAT GetFormat(Handle handle);

size_t GetSamples(Handle handle);

Type GetType(Handle handle);

size_t GetSlotCount(Handle handle);

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShaderResourceView(Handle handle);

}  // namespace Texture
}  // namespace Rendering
