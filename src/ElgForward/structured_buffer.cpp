#include "structured_buffer.h"

#include <malloc.h>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "resource_array.h"
#include "handle_cache.h"

namespace StructuredBuffer {

class Storage {
public:
  Storage(size_t element_size, size_t element_align, size_t max_size)
    : m_element_size_(element_size),
      m_element_align_(element_align),
      m_max_size_(max_size),
      m_current_size_(0),
      m_cpu_buffer_(_aligned_malloc(element_size * max_size, element_align)) {
  }

  bool Initialize(void* initial_data, size_t initial_size, ID3D11Device* device) {
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = m_element_size_ * m_max_size_;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = m_element_size_;

    HRESULT cb_result;
    if (initial_data != nullptr && initial_size > 0) {
      m_current_size_ = initial_size;
      std::memcpy(m_cpu_buffer_.get(), initial_data, m_element_size_ * m_current_size_);

      D3D11_SUBRESOURCE_DATA data;
      data.pSysMem = m_cpu_buffer_.get();
      data.SysMemPitch = 0;
      data.SysMemSlicePitch = 0;

      cb_result = device->CreateBuffer(&desc, &data, m_gpu_buffer_.GetAddressOf());
    } else {
      cb_result = device->CreateBuffer(&desc, nullptr, m_gpu_buffer_.GetAddressOf());
    }

    if (FAILED(cb_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, cb_result);
      return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = DXGI_FORMAT_UNKNOWN;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srv_desc.Buffer.FirstElement = 0;
    srv_desc.Buffer.NumElements = m_max_size_;

    auto srv_result = device->CreateShaderResourceView(m_gpu_buffer_.Get(), &srv_desc, m_srv_.GetAddressOf());
    if (FAILED(srv_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, srv_result);
      return false;
    }

    return true;
  }

  void* GetCpuBuffer() {
    return m_cpu_buffer_.get();
  }

  void* GetElementAt(size_t index) {
    size_t offset = index * m_element_size_;
    return static_cast<char*>(m_cpu_buffer_.get()) + offset;
  }

  ID3D11Buffer* GetGpuBuffer() {
    return m_gpu_buffer_.Get();
  }

  ID3D11Buffer** GetAddressOfGpuBuffer() {
    return m_gpu_buffer_.GetAddressOf();
  }

  ID3D11ShaderResourceView* GetShaderResourceView() {
    return m_srv_.Get();
  }

  ID3D11ShaderResourceView** GetAddressOfShaderResourceView() {
    return m_srv_.GetAddressOf();
  }

  void Resize(size_t new_size) {
    if (new_size < m_max_size_) {
      m_current_size_ = new_size;
    } else {
      DXFW_TRACE(__FILE__, __LINE__, true, "Attempted to resize to %d elements on a structured buffer with max size %d", new_size, m_max_size_);
    }
  }

  size_t GetCurrentSize() const {
    return m_current_size_;
  }

  size_t GetMaxSize() const {
    return m_max_size_;
  }

  bool SendToGpu(ID3D11DeviceContext* device_context) {
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;

    auto map_result = device_context->Map(m_gpu_buffer_.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    if (FAILED(map_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, map_result);
      return false;
    }

    memcpy(mapped_subresource.pData, m_cpu_buffer_.get(), m_current_size_ * m_element_size_);

    device_context->Unmap(m_gpu_buffer_.Get(), 0);

    return true;
  }

private:
  struct CpuBufferDeleter {
    void operator()(void* ptr) {
      _aligned_free(ptr);
    }
  };

  size_t m_element_size_ = 0;
  size_t m_element_align_ = 0;
  size_t m_max_size_ = 0;
  size_t m_current_size_ = 0;
  std::unique_ptr<void, CpuBufferDeleter> m_cpu_buffer_ = {};
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_gpu_buffer_ = {};
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv_ = {};
};

ResourceArray<StructuredBufferHandle, Storage, 255> g_storage_;
HandleCache<size_t, StructuredBufferHandle> g_cache_;

StructuredBufferHandle Create(
  size_t name_hash,
  size_t type_hash,
  size_t type_size,
  size_t type_alignment,
  size_t max_size,
  void* initial_data,
  size_t initial_size,
  ID3D11Device* device) {
  auto cache_key = name_hash;
  hash_combine(cache_key, type_hash);
  hash_combine(cache_key, max_size);

  auto cached_handle = g_cache_.Get(cache_key);
  if (cached_handle.IsValid()) {
    return cached_handle;
  }

  Storage storage(type_size, type_alignment, max_size);
  bool init_ok = storage.Initialize(initial_data, initial_size, device);
  if (!init_ok) {
    return{};
  }

  auto new_handle = g_storage_.Add(std::move(storage));
  g_cache_.Set(cache_key, new_handle);
  return new_handle;
}

void* GetCpuBuffer(StructuredBufferHandle handle) {
  return g_storage_.Get(handle).GetCpuBuffer();
}

void* GetElementAt(StructuredBufferHandle handle, size_t index) {
  return g_storage_.Get(handle).GetElementAt(index);
}

ID3D11Buffer* GetGpuBuffer(StructuredBufferHandle handle) {
  return g_storage_.Get(handle).GetGpuBuffer();
}

ID3D11Buffer** GetAddressOfGpuBuffer(StructuredBufferHandle handle) {
  return g_storage_.Get(handle).GetAddressOfGpuBuffer();
}

ID3D11ShaderResourceView* GetShaderResourceView(StructuredBufferHandle handle) {
  return g_storage_.Get(handle).GetShaderResourceView();
}

ID3D11ShaderResourceView** GetAddressOfShaderResourceView(StructuredBufferHandle handle) {
  return g_storage_.Get(handle).GetAddressOfShaderResourceView();
}

void ResizeStructuredBuffer(StructuredBufferHandle handle, size_t new_size) {
  g_storage_.Get(handle).Resize(new_size);
}

size_t GetCurrentSize(StructuredBufferHandle handle) {
  return g_storage_.Get(handle).GetCurrentSize();
}

size_t GetMaxSize(StructuredBufferHandle handle) {
  return g_storage_.Get(handle).GetMaxSize();
}

bool SendToGpu(StructuredBufferHandle handle, ID3D11DeviceContext* device_context) {
  return g_storage_.Get(handle).SendToGpu(device_context);
}

}  // namespace StructuredBuffer