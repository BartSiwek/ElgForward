#include "constant_buffer.h"

#include <malloc.h>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "core/resource_array.h"
#include "core/handle_cache.h"

namespace Rendering {
namespace ConstantBuffer {

struct ConstantBufferGpuBufferTag {};

using GpuBufferHandle = Core::Handle<8, 24, ConstantBufferGpuBufferTag>;

struct GpuStorage {
 public:
  GpuStorage() = default;
  ~GpuStorage() = default;

  GpuStorage(const GpuStorage&) = delete;
  GpuStorage& operator=(const GpuStorage&) = delete;

  GpuStorage(GpuStorage&&) = default;
  GpuStorage& operator=(GpuStorage&&) = default;

  bool Initialize(size_t size, void* initial_data, ID3D11Device* device) {
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = size;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    HRESULT cb_result;
    if (initial_data != nullptr) {
      D3D11_SUBRESOURCE_DATA data;
      data.pSysMem = initial_data;
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

    return true;
  }

  Microsoft::WRL::ComPtr<ID3D11Buffer> GetGpuBuffer() {
    return m_gpu_buffer_;
  }

  bool SendToGpu(void* data, size_t size, ID3D11DeviceContext* device_context) {
    if (data == nullptr) {
      return false;
    }

    D3D11_MAPPED_SUBRESOURCE mapped_subresource;

    auto map_result = device_context->Map(m_gpu_buffer_.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    if (FAILED(map_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, map_result);
      return false;
    }

    memcpy(mapped_subresource.pData, data, size);

    device_context->Unmap(m_gpu_buffer_.Get(), 0);

    return true;
  }

 private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_gpu_buffer_ = {};
};

struct CpuStorage {
public:
  CpuStorage(size_t size, size_t align, void* initial_data, GpuBufferHandle handle) 
    : m_size_(size),
      m_align_(align),
      m_cpu_buffer_(_aligned_malloc(size, align)),
      m_gpu_handle_(handle) {
    if (initial_data != nullptr) {
      std::memcpy(m_cpu_buffer_.get(), initial_data, m_size_);
    }
  }

  ~CpuStorage() = default;

  CpuStorage(const CpuStorage&) = delete;
  CpuStorage& operator=(const CpuStorage&) = delete;

  CpuStorage(CpuStorage&&) = default;
  CpuStorage& operator=(CpuStorage&&) = default;

  size_t GetSize() const {
    return m_size_;
  }

  size_t GetAlign() const {
    return m_align_;
  }

  void* GetCpuBuffer() const {
    return m_cpu_buffer_.get();
  }

  GpuBufferHandle GetGpuBufferHandle() const {
    return m_gpu_handle_;
  }

private:
  struct CpuBufferDeleter {
    void operator()(void* ptr) {
      _aligned_free(ptr);
    }
  };

  size_t m_size_ = 0;
  size_t m_align_ = 0;
  std::unique_ptr<void, CpuBufferDeleter> m_cpu_buffer_ = {};
  GpuBufferHandle m_gpu_handle_;
};

Core::ResourceArray<GpuBufferHandle, GpuStorage, 255> g_gpu_storage_;
Core::HandleCache<size_t, GpuBufferHandle> g_gpu_cache_;

Core::ResourceArray<Handle, CpuStorage, 255> g_cpu_storage_;
Core::HandleCache<size_t, Handle> g_cpu_cache_;

GpuBufferHandle CreateGpuBuffer(size_t name_hash, size_t type_hash, size_t type_size, void* initial_data, ID3D11Device* device) {
  auto cache_key = name_hash;
  hash_combine(name_hash, type_hash);

  auto cached_handle = g_gpu_cache_.Get(name_hash);
  if (cached_handle.IsValid()) {
    return cached_handle;
  }

  GpuStorage storage;
  bool init_ok = storage.Initialize(type_size, initial_data, device);
  if (!init_ok) {
    return {};
  }

  auto new_handle = g_gpu_storage_.Add(std::move(storage));
  g_gpu_cache_.Set(cache_key, new_handle);
  return new_handle;
}

Handle Create(size_t cpu_name_hash, size_t gpu_name_hash, size_t type_hash, size_t type_size, size_t type_alignment, void* initial_data, ID3D11Device* device) {
  auto cache_key = cpu_name_hash;
  hash_combine(cache_key, type_hash);

  auto cached_handle = g_cpu_cache_.Get(cache_key);
  if (cached_handle.IsValid()) {
    return cached_handle;
  }

  auto gpu_handle = CreateGpuBuffer(gpu_name_hash, type_hash, type_size, initial_data, device);

  CpuStorage storage(type_size, type_alignment, initial_data, gpu_handle);

  auto new_handle = g_cpu_storage_.Add(std::move(storage));
  g_cpu_cache_.Set(cache_key, new_handle);
  return new_handle;
}

Handle Create(const std::string& cpu_name, const std::string& gpu_name, size_t type_hash, size_t type_size, size_t type_alignment, void* initial_data, ID3D11Device* device) {
  std::hash<std::string> hasher;
  return Create(hasher(cpu_name), hasher(gpu_name), type_hash, type_size, type_alignment, initial_data, device);
}

Handle Create(size_t name_hash, size_t type_hash, size_t type_size, size_t type_alignment, void* initial_data, ID3D11Device* device) {
  return Create(name_hash, name_hash, type_hash, type_size, type_alignment, initial_data, device);
}

Handle Create(const std::string& name, size_t type_hash, size_t type_size, size_t type_alignment, void* initial_data, ID3D11Device* device) {
  std::hash<std::string> hasher;
  return Create(hasher(name), type_hash, type_size, type_alignment, initial_data, device);
}

void* GetCpuBuffer(Handle handle) {
  return g_cpu_storage_.Get(handle).GetCpuBuffer();
}

Microsoft::WRL::ComPtr<ID3D11Buffer> GetGpuBuffer(Handle handle) {
  auto gpu_handle = g_cpu_storage_.Get(handle).GetGpuBufferHandle();
  return g_gpu_storage_.Get(gpu_handle).GetGpuBuffer();
}

bool SendToGpu(Handle handle, ID3D11DeviceContext* device_context) {
  auto& cpu_storage = g_cpu_storage_.Get(handle);

  auto size = cpu_storage.GetSize();
  auto data = cpu_storage.GetCpuBuffer();
  auto gpu_handle = cpu_storage.GetGpuBufferHandle();

  return g_gpu_storage_.Get(gpu_handle).SendToGpu(data, size, device_context);  
}

}  // namespace ConstantBuffer
}  // namespace Rendering