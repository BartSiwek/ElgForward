#include "index_buffer.h"

#include <vector>

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "core/resource_array.h"
#include "core/handle_cache.h"

namespace Rendering {
namespace IndexBuffer {

Core::ResourceArray<Handle, Microsoft::WRL::ComPtr<ID3D11Buffer>, 255> g_storage_;
Core::HandleCache<size_t, Handle> g_cache_;

Handle Create(size_t hash, const void* data, size_t data_size, ID3D11Device* device) {
  auto cached_handle = g_cache_.Get(hash);
  if (cached_handle.IsValid()) {
    return cached_handle;
  }

  D3D11_BUFFER_DESC bufferDesc;
  ZeroMemory(&bufferDesc, sizeof(bufferDesc));

  bufferDesc.Usage = D3D11_USAGE_DEFAULT;
  bufferDesc.ByteWidth = static_cast<UINT>(data_size);
  bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bufferDesc.CPUAccessFlags = 0;
  bufferDesc.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA bufferData;
  ZeroMemory(&bufferData, sizeof(bufferData));
  bufferData.pSysMem = data;

  Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
  HRESULT create_buffer_result = device->CreateBuffer(&bufferDesc, &bufferData, buffer.GetAddressOf());

  if (FAILED(create_buffer_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, create_buffer_result);
    return {};
  }

  auto new_handle = g_storage_.Add(buffer);
  g_cache_.Set(hash, new_handle);
  return new_handle;
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Retreive(Handle handle) {
  return g_storage_.Get(handle);
}

}  // namespace IndexBuffer
}  // namespace Rendering
