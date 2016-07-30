#include "rendering/drawable.h"

namespace Rendering {

int32_t GetVertexBufferIndex(VertexDataChannel channel, const Mesh::MeshData& mesh) {
  auto it = std::find(std::begin(mesh.VertexDataChannels), std::end(mesh.VertexDataChannels), channel);
  if (it != std::end(mesh.VertexDataChannels)) {
    int32_t index = std::distance(std::begin(mesh.VertexDataChannels), it);
    return index;
  }
  return -1;
}

bool IsVertexBufferFormatCompatible(uint32_t component_count, D3D_REGISTER_COMPONENT_TYPE component_type, DXGI_FORMAT mesh_channel_format) {
  static std::unordered_map<DXGI_FORMAT, std::pair<uint32_t, D3D_REGISTER_COMPONENT_TYPE>> type_map = {
    { DXGI_FORMAT_R32_FLOAT, { 1, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R32G32_FLOAT, { 2, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R32G32B32_FLOAT, { 3, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R32G32B32A32_FLOAT, { 4, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R16_FLOAT, { 1, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R16G16_FLOAT, { 2, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R16G16B16A16_FLOAT, { 4, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R11G11B10_FLOAT, { 3, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R32_UINT, { 1, D3D10_REGISTER_COMPONENT_UINT32 } },
    { DXGI_FORMAT_R32G32_UINT, { 2, D3D10_REGISTER_COMPONENT_UINT32 } },
    { DXGI_FORMAT_R32G32B32_UINT, { 3, D3D10_REGISTER_COMPONENT_UINT32 } },
    { DXGI_FORMAT_R32G32B32A32_UINT, { 4, D3D10_REGISTER_COMPONENT_UINT32 } },
    { DXGI_FORMAT_R16_UINT, { 1, D3D10_REGISTER_COMPONENT_UINT32 } },
    { DXGI_FORMAT_R16G16_UINT, { 2, D3D10_REGISTER_COMPONENT_UINT32 } },
    { DXGI_FORMAT_R16G16B16A16_UINT, { 4, D3D10_REGISTER_COMPONENT_UINT32 } },
    { DXGI_FORMAT_R8_UINT, { 1, D3D10_REGISTER_COMPONENT_UINT32 } },
    { DXGI_FORMAT_R8G8_UINT, { 2, D3D10_REGISTER_COMPONENT_UINT32 } },
    { DXGI_FORMAT_R8G8B8A8_UINT, { 4, D3D10_REGISTER_COMPONENT_UINT32 } },
    { DXGI_FORMAT_R10G10B10A2_UINT, { 4, D3D10_REGISTER_COMPONENT_UINT32 } },
    { DXGI_FORMAT_R32_SINT, { 1, D3D10_REGISTER_COMPONENT_SINT32 } },
    { DXGI_FORMAT_R32G32_SINT, { 2, D3D10_REGISTER_COMPONENT_SINT32 } },
    { DXGI_FORMAT_R32G32B32_SINT, { 3, D3D10_REGISTER_COMPONENT_SINT32 } },
    { DXGI_FORMAT_R32G32B32A32_SINT, { 4, D3D10_REGISTER_COMPONENT_SINT32 } },
    { DXGI_FORMAT_R16_SINT, { 1, D3D10_REGISTER_COMPONENT_SINT32 } },
    { DXGI_FORMAT_R16G16_SINT, { 2, D3D10_REGISTER_COMPONENT_SINT32 } },
    { DXGI_FORMAT_R16G16B16A16_SINT, { 4, D3D10_REGISTER_COMPONENT_SINT32 } },
    { DXGI_FORMAT_R8_SINT, { 1, D3D10_REGISTER_COMPONENT_SINT32 } },
    { DXGI_FORMAT_R8G8_SINT, { 2, D3D10_REGISTER_COMPONENT_SINT32 } },
    { DXGI_FORMAT_R8G8B8A8_SINT, { 4, D3D10_REGISTER_COMPONENT_SINT32 } },
    { DXGI_FORMAT_R16_UNORM, { 1, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R16G16_UNORM, { 2, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R16G16B16A16_UNORM, { 4, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R8_UNORM, { 1, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R8G8_UNORM, { 2, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R8G8B8A8_UNORM, { 4, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_B8G8R8A8_UNORM, { 4, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_B8G8R8X8_UNORM, { 4, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R10G10B10A2_UNORM, { 4, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R16_SNORM, { 1, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R16G16_SNORM, { 2, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R16G16B16A16_SNORM, { 4, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R8_SNORM, { 1, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R8G8_SNORM, { 2, D3D_REGISTER_COMPONENT_FLOAT32 } },
    { DXGI_FORMAT_R8G8B8A8_SNORM, { 4, D3D_REGISTER_COMPONENT_FLOAT32 } },
  };

  auto it = type_map.find(mesh_channel_format);
  if (it != std::end(type_map)) {
    if (component_count == it->second.first && component_type == it->second.second) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool CreateDrawable(Mesh::Handle mesh_handle, const Material& material, ID3D11Device* device, Drawable* drawable) {
  auto mesh_ptr = Mesh::Retreive(mesh_handle);
  auto vertex_shader_ptr = Rendering::VertexShader::Retreive(material.VertexShader);
  auto pixel_shader_ptr = Rendering::PixelShader::Retreive(material.PixelShader);
  auto constant_buffer_ptr = Rendering::ConstantBuffer::GetGpuBuffer(material.MaterialConstantBuffer);

  std::vector<D3D11_INPUT_ELEMENT_DESC> input_layout_desc;

  uint32_t slot_index = 0;
  for (const auto& input_desc : vertex_shader_ptr->InputDescription) {
    auto input_index = GetVertexBufferIndex(input_desc.Channel, *mesh_ptr);

    if (input_index == -1) {
      return false;
    }

    bool is_compatible = IsVertexBufferFormatCompatible(input_desc.ComponentCount, input_desc.ComponentType, mesh_ptr->VertexBufferFormats[input_index]);
    if (!is_compatible) {
      return false;
    }

    drawable->SetVertexBuffer(slot_index, mesh_ptr->VertexBuffers[input_index], mesh_ptr->VertexBufferStrides[input_index]);

    input_layout_desc.emplace_back();
    auto& input_layout_desc_entry = input_layout_desc.back();

    input_layout_desc_entry.SemanticName = input_desc.SemanticName.c_str();
    input_layout_desc_entry.SemanticIndex = input_desc.SemanticIndex;
    input_layout_desc_entry.Format = mesh_ptr->VertexBufferFormats[input_index];
    input_layout_desc_entry.InputSlot = slot_index;
    input_layout_desc_entry.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    input_layout_desc_entry.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    input_layout_desc_entry.InstanceDataStepRate = 0;

    ++slot_index;
  }

  bool vertex_layout_ok = drawable->SetVertexLayout(input_layout_desc, vertex_shader_ptr->Buffer.Get(), device);
  if (!vertex_layout_ok) {
    return false;
  }

  bool index_data_ok = drawable->SetIndexData(mesh_ptr->IndexBuffer, mesh_ptr->IndexBufferFormat, mesh_ptr->IndexCount, mesh_ptr->PrimitiveTopology);
  if (!index_data_ok) {
    return false;
  }

  bool vs_ok = drawable->SetVertexShader(vertex_shader_ptr->Shader);
  if (!vs_ok) {
    return false;
  }

  bool ps_ok = drawable->SetPixelShader(pixel_shader_ptr->Shader);
  if (!ps_ok) {
    return false;
  }

  bool material_consntant_buffer_ok = drawable->SetMaterialConstantBuffer(constant_buffer_ptr);
  if (!material_consntant_buffer_ok) {
    return false;
  }

  return true;
}

}  // namespace Rendering
