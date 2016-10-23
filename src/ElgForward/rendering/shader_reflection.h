#pragma once

#include <vector>
#include <unordered_map>

#include <d3d11.h>

#include "vertex_data.h"
#include "texture.h"

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
  TexureDescription(const char* name, Texture::Type type, uint32_t samples, uint32_t channels, uint32_t bind_slot_start, uint32_t bind_slot_count)
    : Name(name),
      Type(type),
      Samples(samples),
      Channels(channels),
      BindSlotStart(bind_slot_start),
      BindSlotCount(bind_slot_count) {
  }

  std::string Name = "";
  Texture::Type Type;
  uint32_t Samples;
  uint32_t Channels;
  uint32_t BindSlotStart;
  uint32_t BindSlotCount;
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
