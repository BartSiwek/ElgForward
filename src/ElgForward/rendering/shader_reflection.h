#pragma once

#include <vector>
#include <unordered_map>

#include <d3d11.h>

#include "vertex_data.h"

namespace Rendering {
namespace ShaderReflection {

struct InputDescription {
  InputDescription(const char* name, uint32_t index, VertexDataChannel channel, uint32_t component_count, D3D_REGISTER_COMPONENT_TYPE component_type)
    : SemanticName(name),
      SemanticIndex(index),
      Channel(channel),
      ComponentCount(component_count),
      ComponentType(component_type) {
  }

  std::string SemanticName = "";
  uint32_t SemanticIndex = 0;
  VertexDataChannel Channel = VertexDataChannel::UNKNOWN;
  uint32_t ComponentCount = 0;
  D3D_REGISTER_COMPONENT_TYPE ComponentType = D3D_REGISTER_COMPONENT_UNKNOWN;
};

struct TexureDescription {
  TexureDescription(const char* name, uint32_t component_count)
    : Name(name),
      ComponentCount(component_count) {
  }

  std::string Name = "";
  uint32_t ComponentCount = 0;
};

struct ReflectionData {
  ReflectionData() = default;
  ~ReflectionData() = default;

  ReflectionData(const ReflectionData&) = delete;
  ReflectionData& operator=(const ReflectionData&) = delete;

  ReflectionData(ReflectionData&&) = default;
  ReflectionData& operator=(ReflectionData&&) = default;

  std::vector<InputDescription> Inputs = {};
  std::vector<TexureDescription> Texures = {};
};

bool ReflectInputs(ID3DBlob* blob, const std::unordered_map<std::string, VertexDataChannel>& custom_channel_map, ReflectionData* output);

bool ReflectTextures(ID3DBlob* blob, ReflectionData* output);

}  // namespace ShaderReflection
}  // namespace Rendering
