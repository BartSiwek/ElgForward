#include "rendering/shader_reflection.h"

#include <d3d11.h>
#include <D3Dcompiler.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include <sstream>

namespace Rendering {
namespace ShaderReflection {

const std::unordered_map<std::string, VertexDataChannel>& GetStandardChannelMap() {
  static std::unordered_map<std::string, VertexDataChannel> standard_channel_map = {
    { "POSITION0", VertexDataChannel::POSITIONS },
    { "NORMAL0", VertexDataChannel::NORMALS },
    { "TANGENT0", VertexDataChannel::TANGENTS },
    { "BITANGENT0", VertexDataChannel::BITANGENTS },
    { "TEXCOORD0", VertexDataChannel::TEXCOORDS0 },
    { "TEXCOORD1", VertexDataChannel::TEXCOORDS1 },
    { "TEXCOORD2", VertexDataChannel::TEXCOORDS2 },
    { "TEXCOORD3", VertexDataChannel::TEXCOORDS3 },
    { "TEXCOORD4", VertexDataChannel::TEXCOORDS4 },
    { "TEXCOORD5", VertexDataChannel::TEXCOORDS5 },
    { "TEXCOORD6", VertexDataChannel::TEXCOORDS6 },
    { "TEXCOORD7", VertexDataChannel::TEXCOORDS7 },
    { "COLOR0", VertexDataChannel::COLORS0 },
    { "COLOR1", VertexDataChannel::COLORS1 },
    { "COLOR2", VertexDataChannel::COLORS2 },
    { "COLOR3", VertexDataChannel::COLORS3 },
    { "COLOR4", VertexDataChannel::COLORS4 },
    { "COLOR5", VertexDataChannel::COLORS5 },
    { "COLOR6", VertexDataChannel::COLORS6 },
    { "COLOR7", VertexDataChannel::COLORS7 },
  };

  return standard_channel_map;
}

uint32_t GetComponentCount(BYTE mask) {
  uint32_t component_count = 0;
  while (mask != 0) {
    if (mask & 0x01) {
      component_count++;
    }
    mask = mask >> 1;
  }
  return component_count;
}

bool MapSemanticsToChannel(const char* semantic_name, uint32_t index, const std::unordered_map<std::string, VertexDataChannel>& custom_channel_map, VertexDataChannel* channel) {
  std::string full_name(semantic_name);
  full_name.append(std::to_string(index));

  auto custom_channel_it = custom_channel_map.find(full_name);
  if (custom_channel_it != std::end(custom_channel_map)) {
    *channel = custom_channel_it->second;
    return true;
  }

  auto standard_channel_map = GetStandardChannelMap();
  auto standard_channel_it = standard_channel_map.find(full_name);
  if (standard_channel_it != std::end(standard_channel_map)) {
    *channel = standard_channel_it->second;
    return true;
  }

  return false;
}

bool ReflectInputs(ID3DBlob* blob, const std::unordered_map<std::string, VertexDataChannel>& custom_channel_map, ReflectionData* output) {
  if (output == nullptr) {
    return false;
  }

  Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflector;
  HRESULT reflector_creation_result = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)reflector.GetAddressOf());
  if (FAILED(reflector_creation_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, reflector_creation_result);
    return {};
  }

  D3D11_SHADER_DESC shader_desc;
  HRESULT get_desc_result = reflector->GetDesc(&shader_desc);
  if (FAILED(get_desc_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, get_desc_result);
    return false;
  }

  D3D11_SIGNATURE_PARAMETER_DESC param_desc;
  for (uint32_t i = 0; i < shader_desc.InputParameters; ++i) {
    reflector->GetInputParameterDesc(i, &param_desc);

    auto component_count = GetComponentCount(param_desc.Mask);
    VertexDataChannel channel;
    bool map_ok = MapSemanticsToChannel(param_desc.SemanticName, param_desc.SemanticIndex, custom_channel_map, &channel);
    if (!map_ok) {
      return false;
    }

    
    output->Inputs.emplace_back(param_desc.SemanticName, param_desc.SemanticIndex, channel, component_count, param_desc.ComponentType);
  }
  
  return true;
}

bool ReflectTextures(ID3DBlob* blob, ReflectionData* output) {
  if (output == nullptr) {
    return false;
  }

  Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflector;
  HRESULT reflector_creation_result = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)reflector.GetAddressOf());
  if (FAILED(reflector_creation_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, reflector_creation_result);
    return{};
  }

  D3D11_SHADER_DESC shader_desc;
  HRESULT get_desc_result = reflector->GetDesc(&shader_desc);
  if (FAILED(get_desc_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, true, get_desc_result);
    return false;
  }

  D3D11_SHADER_INPUT_BIND_DESC bind_desc;
  for (uint32_t i = 0; i < shader_desc.ConstantBuffers; ++i) {
    reflector->GetResourceBindingDesc(i, &bind_desc);

    DXFW_TRACE(__FILE__, __LINE__, false, "%d - %d %d %d %S %d %d %d %d",
               i,
               bind_desc.BindCount,
               bind_desc.BindPoint,
               bind_desc.Dimension,
               bind_desc.Name,
               bind_desc.NumSamples,
               bind_desc.ReturnType,
               bind_desc.Type,
               bind_desc.uFlags);

    if (bind_desc.Type == D3D_SIT_TEXTURE) {
    }
  }

  return true;
}

}  // namespace ShaderReflection
}  // namespace Rendering
