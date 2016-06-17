#pragma once

#include <memory>

#include <d3d11.h>

#include <dxfw/dxfw.h>

template<typename BufferEntryType>
class StructuredBuffer {
 public:
  StructuredBuffer(size_t max_size)
      : m_max_size_(max_size), m_current_size_(0), m_cpu_buffer_(std::make_unique<BufferEntryType[]>(max_size)) {
  }

  StructuredBuffer(size_t max_size, size_t current_elements, std::unique_ptr<BufferEntryType[]>&& cpu_buffer)
      : m_max_size_(max_size), m_current_size_(current_elements), m_cpu_buffer_(std::move(cpu_buffer)) {
  }

  bool Initialize(ID3D11Device* device) {
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = m_max_size_ * sizeof(BufferEntryType);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = sizeof(BufferEntryType);

    HRESULT cb_result;
    if (m_cpu_buffer_) {
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

    return true;
  }

  bool SendToGpu(ID3D11DeviceContext* device_context) {
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;

    auto map_result = device_context->Map(m_gpu_buffer_.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    if (FAILED(map_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, map_result);
      return false;
    }

    memcpy(mapped_subresource.pData, m_cpu_buffer_.get(), m_current_size_ * sizeof(BufferEntryType));

    device_context->Unmap(m_gpu_buffer_.Get(), 0);

    return true;
  }

  void Resize(size_t new_size) {
    if (new_size < m_max_size_) {
      m_current_size_ = new_size;
    } else {
      DXFW_TRACE(__FILE__, __LINE__, true, "Attempted to resize to %d elements on a structured buffer with max size %d", new_size, m_max_size_);
    }
  }

  const BufferEntryType& operator[](size_t index) const {
    return m_cpu_buffer_[index];
  }

  BufferEntryType& operator[](size_t index) {
    return m_cpu_buffer_[index];
  }

  const BufferEntryType* GetCpuBuffer() const {
    return m_cpu_buffer_.get();
  }

  BufferEntryType* GetCpuBuffer() {
    return m_cpu_buffer_.get();
  }

  const ID3D11Buffer* GetGpuBuffer() const {
    return m_gpu_buffer_.Get();
  }

  ID3D11Buffer* GetGpuBuffer() {
    return m_gpu_buffer_.Get();
  }

  const ID3D11Buffer** GetAddressOfGpuBuffer() const {
    return m_gpu_buffer_.GetAddressOf();
  }

  ID3D11Buffer** GetAddressOfGpuBuffer() {
    return m_gpu_buffer_.GetAddressOf();
  }

  BufferEntryType* begin() {
    return m_cpu_buffer_.get();
  }

  BufferEntryType* end() {
    return m_cpu_buffer_.get() + m_current_size_;
  }

  const BufferEntryType* cbegin() const {
    return m_cpu_buffer_.get();
  }

  const BufferEntryType* cend() const {
    std::vector<int> x;
    x.emplace_back(1);

    return m_cpu_buffer_.get() + m_current_size_;
  }

 private:
  size_t m_max_size_ = 0;
  size_t m_current_size_ = 0;
  std::unique_ptr<BufferEntryType[]> m_cpu_buffer_ = {};
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_gpu_buffer_ = {};
};
