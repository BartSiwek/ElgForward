#include "constant_buffer.h"

#include <malloc.h>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "resource_array.h"
#include "handle_cache.h"

struct ConstantBufferStorage {
public:
  ConstantBufferStorage(size_t size, size_t align) : m_size_(size), m_align_(align), m_cpu_buffer_(_aligned_malloc(size, align)) {
  }

  ConstantBufferStorage(size_t size, size_t align, void* initial_data) : ConstantBufferStorage(size, align) {
    if (initial_data != nullptr) {
      std::memcpy(m_cpu_buffer_.get(), initial_data, size);
    }
  }

  ~ConstantBufferStorage() = default;

  ConstantBufferStorage(const ConstantBufferStorage&) = delete;
  ConstantBufferStorage& operator=(const ConstantBufferStorage&) = delete;

  ConstantBufferStorage(ConstantBufferStorage&&) = default;
  ConstantBufferStorage& operator=(ConstantBufferStorage&&) = default;

  bool Initialize(ID3D11Device* device) {
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = m_size_;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = m_cpu_buffer_.get();
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;

    HRESULT cb_result = device->CreateBuffer(&desc, &data, m_gpu_buffer_.GetAddressOf());

    if (FAILED(cb_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, cb_result);
      return false;
    }

    return true;
  }

  bool SendToGpu(ID3D11DeviceContext* device_context) {
    if (!m_cpu_buffer_) {
      return false;
    }

    D3D11_MAPPED_SUBRESOURCE mapped_subresource;

    auto map_result = device_context->Map(m_gpu_buffer_.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    if (FAILED(map_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, map_result);
      return false;
    }

    memcpy(mapped_subresource.pData, m_cpu_buffer_.get(), m_size_);

    device_context->Unmap(m_gpu_buffer_.Get(), 0);

    return true;
  }

  void* GetCpuBuffer() {
    return m_cpu_buffer_.get();
  }

  ID3D11Buffer* GetGpuBuffer() {
    return m_gpu_buffer_.Get();
  }

  ID3D11Buffer** GetAddressOfGpuBuffer() {
    return m_gpu_buffer_.GetAddressOf();
  }

private:
  struct CpuBufferDeleter {
    void operator()(void* ptr) {
      _aligned_free(ptr);
    }
  };

  size_t m_size_;
  size_t m_align_;
  std::unique_ptr<void, CpuBufferDeleter> m_cpu_buffer_ = {};
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_gpu_buffer_ = {};
};

ResourceArray<ConstantBufferHandle, ConstantBufferStorage, 255> g_storage_;
HandleCache<size_t, ConstantBufferHandle> g_cache_;

ConstantBufferHandle CreateConstantBuffer(size_t name_hash, size_t type_hash, size_t type_size, size_t type_alignment, void* initial_data, ID3D11Device* device) {
  auto cache_key = name_hash;
  hash_combine(cache_key, type_hash);

  auto cached_handle = g_cache_.Get(cache_key);
  if (cached_handle.IsValid()) {
    return cached_handle;
  }

  ConstantBufferStorage storage(type_size, type_alignment, initial_data);
  bool init_ok = storage.Initialize(device);
  if (!init_ok) {
    return {};
  }

  auto new_handle = g_storage_.Add(std::move(storage));
  g_cache_.Set(cache_key, new_handle);
  return new_handle;
}

bool InitializeConstantBuffer(ConstantBufferHandle handle, ID3D11Device* device) {
  return g_storage_.Get(handle).Initialize(device);
}

void* GetCpuBuffer(ConstantBufferHandle handle) {
  return g_storage_.Get(handle).GetCpuBuffer();
}

ID3D11Buffer* GetGpuBuffer(ConstantBufferHandle handle) {
  return g_storage_.Get(handle).GetGpuBuffer();
}

ID3D11Buffer** GetAddressOfGpuBuffer(ConstantBufferHandle handle) {
  return g_storage_.Get(handle).GetAddressOfGpuBuffer();
}
bool SendToGpu(ConstantBufferHandle handle, ID3D11DeviceContext* device_context) {
  return g_storage_.Get(handle).SendToGpu(device_context);
}
