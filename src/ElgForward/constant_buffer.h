#pragma once

#include <d3d11.h>

#include <dxfw/dxfw.h>

template<typename BackingBufferType>
struct ConstantBuffer {
 public:
  ConstantBuffer() : m_cpu_buffer_(std::make_unique<BackingBufferType>()) {
  }

  ConstantBuffer(std::unique_ptr<BackingBufferType>&& cpu_buffer) : m_cpu_buffer_(std::move(cpu_buffer)) {
  }

  bool Initialize(ID3D11Device* device) {
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = sizeof(BackingBufferType);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

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

    memcpy(mapped_subresource.pData, m_cpu_buffer_.get(), sizeof(BackingBufferType));

    device_context->Unmap(m_gpu_buffer_.Get(), 0);

    return true;
  }

  const BackingBufferType* GetCpuBuffer() const {
    return m_cpu_buffer_.get();
  }

  BackingBufferType* GetCpuBuffer() {
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

 private:
  std::unique_ptr<BackingBufferType> m_cpu_buffer_ = {};
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_gpu_buffer_ = {};
};
