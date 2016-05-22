#pragma once

#include <d3d11.h>

#include <dxfw/dxfw.h>

template<typename BufferType>
bool CrateConstantBuffer(BufferType* initial, ID3D11Device* device, ID3D11Buffer** constant_buffer) {
  D3D11_BUFFER_DESC desc;
  desc.ByteWidth = sizeof(BufferType);
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  desc.MiscFlags = 0;
  desc.StructureByteStride = 0;

  HRESULT cb_result;
  if (initial != nullptr) {
    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = initial;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;

    cb_result = device->CreateBuffer(&desc, &data, constant_buffer);
  } else {
    cb_result = device->CreateBuffer(&desc, nullptr, constant_buffer);
  }

  if (FAILED(cb_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, cb_result);
    return false;
  }

  return true;
}

template<typename BufferType>
bool UpdateConstantBuffer(BufferType* data, ID3D11DeviceContext* device_context, ID3D11Buffer* constant_buffer) {
  D3D11_MAPPED_SUBRESOURCE mapped_subresource;

  auto map_result = device_context->Map(constant_buffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
  if (FAILED(map_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, map_result);
    return false;
  }

  memcpy(mapped_subresource.pData, data, sizeof(BufferType));

  device_context->Unmap(constant_buffer, 0);

  return true;
}