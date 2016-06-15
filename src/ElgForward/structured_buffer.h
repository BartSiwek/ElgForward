#pragma once

#include <d3d11.h>

#include <dxfw/dxfw.h>

template<typename BufferEntryType>
class StructuredBuffer {
 public:
  StructuredBuffer(size_t max_elements)
      : m_max_elements_(max_elements), m_current_elements_(0), m_cpu_buffer_(std::make_unique<BufferEntryType[]>(max_elements)) {
  }

  StructuredBuffer(size_t max_elements, size_t current_elements, std::unique_ptr<BufferEntryType[]>&& cpu_buffer)
      : m_max_elements_(max_elements), m_current_elements_(current_elements), m_cpu_buffer_(std::move(cpu_buffer)) {
  }

  bool Initialize(ID3D11Device* device) {
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = m_max_elements_ * sizeof(BufferEntryType);
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

  bool Update(ID3D11DeviceContext* device_context) {
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;

    auto map_result = device_context->Map(m_gpu_buffer_.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    if (FAILED(map_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, map_result);
      return false;
    }

    memcpy(mapped_subresource.pData, m_cpu_buffer_.get(), m_current_elements_ * sizeof(BufferEntryType));

    device_context->Unmap(m_gpu_buffer_.Get(), 0);

    return true;
  }

  void SetCurrentElementCount(size_t current_elements) {
    if (current_elements < m_max_elements_) {
      m_current_elements_ = current_elements;
    } else {
      DXFW_TRACE(__FILE__, __LINE__, true, "Attempted to set %d elements on a structured buffer with max size %d", current_elements, m_current_elements_);
    }
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

 private:
  size_t m_max_elements_ = 0;
  size_t m_current_elements_ = 0;
  std::unique_ptr<BufferEntryType> m_cpu_buffer_ = {};
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_gpu_buffer_ = {};
};
