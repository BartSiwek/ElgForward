#include "texture.h"

#include <vector>

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "core/hash.h"
#include "core/resource_array.h"
#include "core/handle_cache.h"

namespace Rendering {
namespace Texture {

class Storage {
 public:
  Storage(DXGI_FORMAT format, size_t samples, Type type, size_t slot_count)
      : m_format_(format),
        m_samples_(samples),
        m_type_(type),
        m_slot_count_(slot_count) {
  }

  ~Storage() = default;

  Storage(const Storage& other) = delete;
  Storage& operator=(const Storage& other) = delete;

  Storage(Storage&& other) = default;
  Storage& operator=(Storage&& other) = default;

  DXGI_FORMAT GetFormat() const {
    return m_format_;
  }

  size_t GetSamples() const {
    return m_samples_;
  }

  Type GetType() const {
    return m_type_;
  }

  size_t GetSlotCount() const {
    return m_slot_count_;
  }

  const Microsoft::WRL::ComPtr<ID3D11Texture2D>& GetTexture() const {
    return m_texture_;
  }

  Microsoft::WRL::ComPtr<ID3D11Texture2D>& GetTexture() {
    return m_texture_;
  }

  const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetView() const {
    return m_view_;
  }

  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetView() {
    return m_view_;
  }

 private:
  DXGI_FORMAT m_format_ = DXGI_FORMAT_UNKNOWN;
  size_t m_samples_ = static_cast<size_t>(-1);
  Type m_type_ = Type::UNKNOWN;
  size_t m_slot_count_ = 1;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture_ = nullptr;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_view_ = nullptr;
};

Core::ResourceArray<Handle, Storage, 255> g_storage_;
Core::HandleCache<size_t, Handle> g_cache_;

Handle Create(size_t name_hash, const std::vector<ImageData>& data, ID3D11Device* device) {
  auto cached_handle = g_cache_.Get(name_hash);

  if (cached_handle.IsValid()) {
    return cached_handle;
  }

  if (data.size() == 0) {
    return {};
  }

  std::vector<D3D11_SUBRESOURCE_DATA> initial_data = {};
  uint32_t width = 0;
  uint32_t height = 0;
  DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

  for (const auto& single_image_data : data) {
    initial_data.emplace_back(D3D11_SUBRESOURCE_DATA {
      single_image_data.Data,
      single_image_data.Components * single_image_data.Width,
      single_image_data.Components * single_image_data.Width * single_image_data.Height
    });

    width = std::max(width, single_image_data.Width);
    height = std::max(height, single_image_data.Height);
    format = single_image_data.Format;
  }

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = initial_data.size();
  desc.ArraySize = 1;
  desc.Format = format;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_IMMUTABLE;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  Storage new_storage(format, static_cast<size_t>(-1), Type::DIM_2, 1);
  auto texture_result = device->CreateTexture2D(&desc, &initial_data[0], new_storage.GetTexture().GetAddressOf());
  if (FAILED(texture_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, texture_result);
    return {};
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
  srv_desc.Format = format;
  srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = desc.MipLevels;
  srv_desc.Texture2D.MostDetailedMip = 0;

  auto view_result = device->CreateShaderResourceView(new_storage.GetTexture().Get(), &srv_desc,
                                                      new_storage.GetView().GetAddressOf());
  if (FAILED(view_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, view_result);
    return {};
  }

  auto new_handle = g_storage_.Add(std::move(new_storage));
  g_cache_.Set(name_hash, new_handle);
  return new_handle;
};

Handle Create(const std::string& name, const std::vector<ImageData>& data, ID3D11Device* device) {
  std::hash<std::string> hasher;
  return Create(hasher(name), data, device);
}

DXGI_FORMAT GetFormat(Handle handle) {
  return g_storage_.Get(handle).GetFormat();
}

size_t GetSamples(Handle handle) {
  return g_storage_.Get(handle).GetSamples();
}

Type GetType(Handle handle) {
  return g_storage_.Get(handle).GetType();
}

size_t GetSlotCount(Handle handle) {
  return g_storage_.Get(handle).GetSlotCount();
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShaderResourceView(Handle handle) {
  return g_storage_.Get(handle).GetView();
}

}  // namespace Texture
}  // namespace Rendering
