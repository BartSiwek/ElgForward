#pragma once

#include <vector>
#include <unordered_map>

#include <d3d11.h>
#include <wrl.h>

#include "core/filesystem.h"
#include "core/handle.h"
#include "rendering/shader_reflection.h"

namespace Rendering {
namespace PixelShader {

struct PixelShaderTag {};

using Handle = Core::Handle<8, 24, PixelShaderTag>;

struct ShaderData {
  ShaderData() = default;
  ~ShaderData() = default;

  ShaderData(const ShaderData&) = delete;
  ShaderData& operator=(const ShaderData&) = delete;

  ShaderData(ShaderData&&) = default;
  ShaderData& operator=(ShaderData&&) = default;

  Microsoft::WRL::ComPtr<ID3DBlob> Buffer = nullptr;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> Shader = nullptr;
  ShaderReflection::ReflectionData ReflectionData = {};
};

Handle Create(const filesystem::path& path, ID3D11Device* device);

ShaderData* Retreive(Handle handle);

}  // namespace PixelShader
}  // namespace Rendering
