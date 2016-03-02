#pragma once

#include <array>

#include "com_helpers.h"
#include "gpu_mesh.h"
#include "material.h"

struct Drawable {
  Drawable() {
    for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i) {
      VertexBuffers[i] = nullptr;
    }
  }

  ~Drawable() {
    for (size_t i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i) {
      SAFE_RELEASE(VertexBuffers[i]);
    }
  }

  std::array<ID3D11Buffer*, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> VertexBuffers;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
};

bool CreateDrawable(const GpuMesh& mesh, const Material& material, ID3D11Device* device);
