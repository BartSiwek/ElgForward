#pragma once

#include <string>

#include <d3d11.h>
#include <wrl.h>

#include "core/handle.h"

namespace Rendering {
namespace ConstantBuffer {

struct ConstantBufferTag {};

using Handle = Core::Handle<8, 24, ConstantBufferTag>;

Handle Create(size_t cpu_name_hash, size_t gpu_name_hash, size_t type_hash, size_t type_size, size_t type_alignment, void* initial_data, ID3D11Device* device);

Handle Create(const std::string& cpu_name, const std::string& gpu_name, size_t type_hash, size_t type_size, size_t type_alignment, void* initial_data, ID3D11Device* device);

Handle Create(size_t name_hash, size_t type_hash, size_t type_size, size_t type_alignment, void* initial_data, ID3D11Device* device);

Handle Create(const std::string& name, size_t type_hash, size_t type_size, size_t type_alignment, void* initial_data, ID3D11Device* device);

void* GetCpuBuffer(Handle handle);

Microsoft::WRL::ComPtr<ID3D11Buffer> GetGpuBuffer(Handle handle);

bool SendToGpu(Handle handle, ID3D11DeviceContext* device_context);

}  // namespace ConstantBuffer
}  // namespace Rendering
