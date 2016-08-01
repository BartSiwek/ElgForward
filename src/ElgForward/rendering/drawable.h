#pragma once

#include <array>

#include <DirectXMath.h>

#include "core/com_array.h"
#include "mesh.h"
#include "material.h"

#include "rendering/vertex_layout.h"
#include "rendering/constant_buffer.h"

namespace Rendering {

class Drawable {
public:
  Drawable() = default;
  ~Drawable() = default;

  Drawable(const Drawable&) = delete;
  Drawable& operator=(const Drawable&) = delete;

  Drawable(Drawable&& other) = default;
  Drawable& operator=(Drawable&& other) = default;

  bool SetVertexBuffer(uint32_t index, Rendering::VertexBuffer::Handle buffer_handle, uint32_t stride) {
    if (buffer_handle.IsValid()) {
      auto buffer = Rendering::VertexBuffer::Retreive(buffer_handle);
      auto buffer_ptr = buffer.Get();

      buffer_ptr->AddRef();

      m_vertex_buffers_[index] = buffer_ptr;
      m_vertex_buffer_strides_[index] = stride;
      m_vertex_buffer_offsets_[index] = 0;

      return true;
    }

    return false;
  }

  ID3D11Buffer* const* GetVertexBuffers() const {
    return &m_vertex_buffers_[0];
  }

  ID3D11Buffer** GetVertexBuffers() {
    return &m_vertex_buffers_[0];
  }

  const uint32_t* GetVertexBufferStrides() const {
    return &m_vertex_buffer_strides_[0];
  }

  uint32_t* GetVertexBufferStrides() {
    return &m_vertex_buffer_strides_[0];
  }

  const uint32_t* GetVertexBufferOffsets() const {
    return &m_vertex_buffer_offsets_[0];
  }

  uint32_t* GetVertexBufferOffsets() {
    return &m_vertex_buffer_offsets_[0];
  }

  bool SetVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC> input_layout_desc, ID3D10Blob* shader_blob, ID3D11Device* device) {
    if (m_input_layout_ == nullptr) {
      auto handle = Rendering::VertexLayout::Create(input_layout_desc, shader_blob, device);
      if (handle.IsValid()) {
        m_input_layout_ = Rendering::VertexLayout::Retreive(handle);
        return true;
      }
    }
    return false;
  }

  ID3D11InputLayout* GetVertexLayout() const {
    return m_input_layout_.Get();
  }

  bool SetIndexData(IndexBuffer::Handle index_buffer_handle, DXGI_FORMAT format, uint32_t index_count, D3D_PRIMITIVE_TOPOLOGY topology) {
    if (m_index_buffer_ == nullptr) {
      m_index_buffer_ = IndexBuffer::Retreive(index_buffer_handle);
      m_index_buffer_format_ = format;
      m_index_count_ = index_count;
      m_primitive_topology_ = topology;
      return true;
    }
    return false;
  }

  ID3D11Buffer* GetIndexBuffer() const {
    return m_index_buffer_.Get();
  }

  DXGI_FORMAT GetIndexBufferFormat() const {
    return m_index_buffer_format_;
  }

  uint32_t GetIndexCount() const {
    return m_index_count_;
  }

  D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const {
    return m_primitive_topology_;
  }

  bool SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vs) {
    if (m_vs_ == nullptr) {
      m_vs_ = vs;
      return true;
    }
    return false;
  }

  ID3D11VertexShader* GetVertexShader() const {
    return m_vs_.Get();
  }

  bool SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> ps) {
    if (m_ps_ == nullptr) {
      m_ps_ = ps;
      return true;
    }
    return false;
  }

  ID3D11PixelShader* GetPixelShader() const {
    return m_ps_.Get();
  }

  bool SetMaterialConstantBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> buffer) {
    if (m_material_constant_buffer_ == nullptr) {
      m_material_constant_buffer_ = buffer;
      return true;
    }
    return false;
  }

  ID3D11Buffer* GetMaterialConstantBuffer() const {
    return m_material_constant_buffer_.Get();
  }

  ID3D11Buffer* const* GetAddressOfMaterialConstantBuffer() const {
    return m_material_constant_buffer_.GetAddressOf();
  }

  ID3D11Buffer** GetAddressOfMaterialConstantBuffer() {
    return m_material_constant_buffer_.GetAddressOf();
  }

  void SetModelMatrix(const DirectX::XMMATRIX& matrix) {
    m_model_matrix_ = matrix;
  }

  const DirectX::XMMATRIX& GetModelMatrix() const {
    return m_model_matrix_;
  }

  DirectX::XMMATRIX& GetModelMatrix() {
    return m_model_matrix_;
  }

  void SetModelMatrixInverseTranspose(const DirectX::XMMATRIX& matrix) {
    m_model_matrix_inverse_transpose_ = matrix;
  }

  const DirectX::XMMATRIX& GetModelMatrixInverseTranspose() const {
    return m_model_matrix_inverse_transpose_;
  }

  DirectX::XMMATRIX& GetModelMatrixInverseTranspose() {
    return m_model_matrix_inverse_transpose_;
  }


private:
  Core::ComArray<ID3D11Buffer*, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> m_vertex_buffers_ = {};
  std::array<uint32_t, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> m_vertex_buffer_strides_ = {};
  std::array<uint32_t, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> m_vertex_buffer_offsets_ = {};
  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_input_layout_ = nullptr;

  Microsoft::WRL::ComPtr<ID3D11Buffer> m_index_buffer_ = nullptr;
  DXGI_FORMAT m_index_buffer_format_ = DXGI_FORMAT_UNKNOWN;
  uint32_t m_index_count_ = 0;
  D3D_PRIMITIVE_TOPOLOGY m_primitive_topology_ = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

  Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs_ = nullptr;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps_ = nullptr;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_material_constant_buffer_ = nullptr;

  DirectX::XMMATRIX m_model_matrix_ = DirectX::XMMatrixIdentity();
  DirectX::XMMATRIX m_model_matrix_inverse_transpose_ = DirectX::XMMatrixIdentity();
};

bool CreateDrawable(Mesh::Handle mesh, const Material& material, ID3D11Device* device, Drawable* drawable);

}  // namespace Rendering
