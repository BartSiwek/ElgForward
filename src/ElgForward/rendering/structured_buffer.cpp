#include "structured_buffer.h"

#include <malloc.h>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "core/resource_array.h"
#include "core/handle_cache.h"

namespace Rendering {
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

  bool SetCurrentSize(size_t new_size) {
    if (new_size > m_max_size_) {
      DXFW_TRACE(__FILE__, __LINE__, true, "Attempted to resize to %d elements on a structured buffer with max size %d", new_size, m_max_size_);
      return false;
    }

    m_current_size_ = new_size;
    return true;
  }

  size_t GetCurrentSize() const {
    return m_current_size_;
  }

  bool Add(void* value) {
    size_t new_size = m_current_size_ + 1;
    
    if (new_size > m_max_size_) {
      DXFW_TRACE(__FILE__, __LINE__, true, "Attempted to add an element to a full structured buffer with max size %d", m_max_size_);
      return false;
    }

    std::memcpy(GetElementAt(m_current_size_), value, m_element_size_);
    m_current_size_ = new_size;
    return true;
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

  const size_t m_element_size_ = 0;
  const size_t m_element_align_ = 0;
  const size_t m_max_size_ = 0;

  size_t m_current_size_ = 0;

  std::unique_ptr<void, CpuBufferDeleter> m_cpu_buffer_ = {};
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_gpu_buffer_ = {};
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv_ = {};
};

Core::ResourceArray<Handle, Storage, 255> g_storage_;
Core::HandleCache<size_t, Handle> g_cache_;

Handle Create(
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

Handle Create(
    const std::string& name,
    size_t type_hash,
    size_t type_size,
    size_t type_alignment,
    size_t max_size,
    void* initial_data,
    size_t initial_count,
    ID3D11Device* device) {
  std::hash<std::string> hasher;
  return Create(hasher(name), type_hash, type_size, type_alignment, max_size, initial_data, initial_count, device);
}

void* GetCpuBuffer(Handle handle) {
  return g_storage_.Get(handle).GetCpuBuffer();
}

void* GetElementAt(Handle handle, size_t index) {
  return g_storage_.Get(handle).GetElementAt(index);
}

ID3D11Buffer* GetGpuBuffer(Handle handle) {
  return g_storage_.Get(handle).GetGpuBuffer();
}

ID3D11Buffer** GetAddressOfGpuBuffer(Handle handle) {
  return g_storage_.Get(handle).GetAddressOfGpuBuffer();
}

ID3D11ShaderResourceView* GetShaderResourceView(Handle handle) {
  return g_storage_.Get(handle).GetShaderResourceView();
}

ID3D11ShaderResourceView** GetAddressOfShaderResourceView(Handle handle) {
  return g_storage_.Get(handle).GetAddressOfShaderResourceView();
}

void SetCurrentSize(Handle handle, size_t new_size) {
  g_storage_.Get(handle).SetCurrentSize(new_size);
}

size_t GetCurrentSize(Handle handle) {
  return g_storage_.Get(handle).GetCurrentSize();
}

bool Add(Handle handle, void* value) {
  return g_storage_.Get(handle).Add(value);
}

size_t GetMaxSize(Handle handle) {
  return g_storage_.Get(handle).GetMaxSize();
}

bool SendToGpu(Handle handle, ID3D11DeviceContext* device_context) {
  return g_storage_.Get(handle).SendToGpu(device_context);
}

}  // namespace StructuredBuffer
}  // namespace Rendering
