#pragma once

#include <string>

#include <d3d11.h>
#include <wrl.h>

#include "core/handle.h"

namespace Rendering {
namespace StructuredBuffer {

struct StructuredBufferTag {};

using Handle = Core::Handle<8, 24, StructuredBufferTag>;

Handle Create(size_t name_hash, size_t type_hash, size_t type_size, size_t type_alignment, size_t max_size,
              void* initial_data, size_t initial_count, ID3D11Device* device);

Handle Create(const std::string& name, size_t type_hash, size_t type_size, size_t type_alignment,
              size_t max_size, void* initial_data, size_t initial_count, ID3D11Device* device);

void* GetCpuBuffer(Handle handle);

void* GetElementAt(Handle handle, size_t index);

Microsoft::WRL::ComPtr<ID3D11Buffer> GetGpuBuffer(Handle handle);

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShaderResourceView(Handle handle);

void SetCurrentSize(Handle handle, size_t new_size);

size_t GetCurrentSize(Handle handle);

bool Add(Handle handle, void* value);

size_t GetMaxSize(Handle handle);

bool SendToGpu(Handle handle, ID3D11DeviceContext* device_context);

}  // namespace StructuredBuffer
}  // namespace Rendering