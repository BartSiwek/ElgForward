#include "gpu_mesh_factory.h"

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "mesh.h"

static_assert(Mesh::PositionType::InputLayoutElementCount == 1, "PositionType has incorrect input element count");
static_assert(Mesh::NormalType::InputLayoutElementCount == 1, "NormalType has incorrect input element count");
static_assert(Mesh::TextureType::InputLayoutElementCount == 1, "TextureType has incorrect input element count");

const std::array<D3D11_INPUT_ELEMENT_DESC, GpuMeshFactory::InputLayoutElementCount> GpuMeshFactory::InputLayout = {
  Mesh::PositionType::InputLayout[0],
  Mesh::NormalType::InputLayout[0],
  Mesh::TextureType::InputLayout[0],
};

template<typename EntryType>
bool CreateVertexBuffer(const std::vector<EntryType>& data, ID3D11Device* device, ID3D11Buffer** buffer) {
  D3D11_BUFFER_DESC bufferDesc;
  ZeroMemory(&bufferDesc, sizeof(bufferDesc));

  bufferDesc.Usage = D3D11_USAGE_DEFAULT;
  bufferDesc.ByteWidth = data.size() * sizeof(EntryType);
  bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bufferDesc.CPUAccessFlags = 0;
  bufferDesc.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA bufferData;
  ZeroMemory(&bufferData, sizeof(bufferData));
  bufferData.pSysMem = &data[0];

  HRESULT create_buffer_result = device->CreateBuffer(&bufferDesc, &bufferData, buffer);

  if (FAILED(create_buffer_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, create_buffer_result, true);
    return false;
  }

  return true;
}

template<typename IndexType>
bool CreateIndexBuffer(const std::vector<IndexType>& data, ID3D11Device* device, ID3D11Buffer** buffer) {
  D3D11_BUFFER_DESC bufferDesc;
  ZeroMemory(&bufferDesc, sizeof(bufferDesc));

  bufferDesc.Usage = D3D11_USAGE_DEFAULT;
  bufferDesc.ByteWidth = data.size() * sizeof(IndexType);
  bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bufferDesc.CPUAccessFlags = 0;
  bufferDesc.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA bufferData;
  ZeroMemory(&bufferData, sizeof(bufferData));
  bufferData.pSysMem = &data[0];

  HRESULT create_buffer_result = device->CreateBuffer(&bufferDesc, &bufferData, buffer);

  if (FAILED(create_buffer_result)) {
    DXFW_DIRECTX_TRACE(__FILE__, __LINE__, create_buffer_result, true);
    return false;
  }

  return true;
}

bool GpuMeshFactory::CreateGpuMesh(const Mesh& mesh, ID3D11Device* device, GpuMesh* gpu_mesh) {
  bool positions_ok = CreateVertexBuffer(mesh.Positions, device, &gpu_mesh->VertexBuffers[GpuMeshFactory::PositionVertexBufferIndex]);
  if (!positions_ok) {
    return false;
  }

  bool normals_ok = CreateVertexBuffer(mesh.Normals, device, &gpu_mesh->VertexBuffers[GpuMeshFactory::NormalVertexBufferIndex]);
  if (!normals_ok) {
    return false;
  }

  bool texture_coords_ok = CreateVertexBuffer(mesh.TextureCoords, device, &gpu_mesh->VertexBuffers[GpuMeshFactory::TextureCoordVertexBufferIndex]);
  if (!texture_coords_ok) {
    return false;
  }

  bool indices_ok = CreateIndexBuffer(mesh.Indices, device, gpu_mesh->IndexBuffer.GetAddressOf());
  if (!indices_ok) {
    return false;
  }

  gpu_mesh->VertexBufferStrides[PositionVertexBufferIndex] = sizeof(Mesh::PositionType);
  gpu_mesh->VertexBufferStrides[NormalVertexBufferIndex] = sizeof(Mesh::NormalType);
  gpu_mesh->VertexBufferStrides[TextureCoordVertexBufferIndex] = sizeof(Mesh::TextureType);

  return true;
}
