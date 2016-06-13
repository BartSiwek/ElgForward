#pragma once

#include <d3d11.h>

#include <dxfw/dxfw.h>

template<typename BackingBufferType>
struct ConstantBuffer {
  ConstantBuffer() : CpuBuffer(std::make_unique<BackingBufferType>()) {
  }

  ConstantBuffer(std::unique_ptr<BackingBufferType>&& cpu_buffer) : CpuBuffer(std::move(cpu_buffer)) {
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
    if (CpuBuffer) {
      D3D11_SUBRESOURCE_DATA data;
      data.pSysMem = CpuBuffer.get();
      data.SysMemPitch = 0;
      data.SysMemSlicePitch = 0;

      cb_result = device->CreateBuffer(&desc, &data, GpuBuffer.GetAddressOf());
    } else {
      cb_result = device->CreateBuffer(&desc, nullptr, GpuBuffer.GetAddressOf());
    }

    if (FAILED(cb_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, cb_result);
      return false;
    }

    return true;
  }

  bool Update(ID3D11DeviceContext* device_context) {
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;

    auto map_result = device_context->Map(GpuBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    if (FAILED(map_result)) {
      DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, map_result);
      return false;
    }

    memcpy(mapped_subresource.pData, CpuBuffer.get(), sizeof(BackingBufferType));

    device_context->Unmap(GpuBuffer.Get(), 0);

    return true;
  }

  std::unique_ptr<BackingBufferType> CpuBuffer = {};
  Microsoft::WRL::ComPtr<ID3D11Buffer> GpuBuffer = {};
};
