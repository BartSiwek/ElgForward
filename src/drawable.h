#pragma once

#include <array>

#include "com_helpers.h"
#include "gpu_mesh.h"
#include "material.h"

#include "vertex_layout_factory.h"

class Drawable {
public:
  Drawable() {
    for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i) {
      m_vertex_buffers_[i] = nullptr;
      m_vertex_buffer_offsets_[i] = 0;
    }
  }

  ~Drawable() {
    for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i) {
      SAFE_RELEASE(m_vertex_buffers_[i]);
    }
  }

  bool SetVertexBuffer(uint32_t index, ID3D11Buffer* buffer) {
    if (m_vertex_buffers_[index] == nullptr) {
      m_vertex_buffers_[index] = buffer;
      buffer->AddRef();
      return true;
    }
    return false;
  }

  const ID3D11Buffer* GetVertexBuffer() const {
    return m_vertex_buffers_[0];
  }

  const uint32_t* GetVertexBufferOffsets() const {
    return &m_vertex_buffer_offsets_[0];
  }

  bool SetVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC> input_layout_desc, ID3D10Blob* shader_blob, ID3D11Device* device) {
    if (m_input_layout_ != nullptr) {
      bool vertex_layout_ok = CreateVertexLayout(input_layout_desc, shader_blob, device, m_input_layout_.GetAddressOf());
      return vertex_layout_ok;
    }
    return false;
  }

  const ID3D11InputLayout* GetInputLayout() const {
    return m_input_layout_.Get();
  }

  bool SetIndexData(Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer, uint32_t index_count, D3D_PRIMITIVE_TOPOLOGY topology) {
    if (m_index_buffer_ == nullptr) {
      m_index_buffer_ = index_buffer;
      m_index_count_ = index_count;
      m_primitive_topology_ = topology;
      return true;
    }
    return false;
  }

  const ID3D11Buffer* GetIndexBuffer() const {
    return m_index_buffer_.Get();
  }

  bool SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vs) {
    if (m_vs_ == nullptr) {
      m_vs_ = vs;
      return true;
    }
    return false;
  }
    
  bool SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> ps) {
    if (m_ps_ == nullptr) {
      m_ps_ = ps;
      return true;
    }
    return false;
  }

private:
  std::array<ID3D11Buffer*, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> m_vertex_buffers_;
  std::array<uint32_t, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> m_vertex_buffer_offsets_;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_input_layout_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> m_index_buffer_ = nullptr;
  uint32_t m_index_count_ = 0;
  D3D_PRIMITIVE_TOPOLOGY m_primitive_topology_ = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

  Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs_ = nullptr;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps_ = nullptr;
};

bool CreateDrawable(const GpuMesh& mesh, const Material& material, ID3D11Device* device, Drawable* drawable);
