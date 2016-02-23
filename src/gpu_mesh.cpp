#include "gpu_mesh.h"

#include <d3d11.h>
#include <wrl.h>

#include <dxfw/dxfw.h>

#include "mesh.h"

const std::array<D3D11_INPUT_ELEMENT_DESC, GpuMesh::MeshLayoutElementCount> GpuMesh::VetexBuffersLayout = GpuMesh::InitializeLayout();

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

bool CreateGpuMesh(const Mesh& mesh, ID3D11Device* device, GpuMesh* gpu_mesh) {
  gpu_mesh->VertexBuffers.resize(GpuMesh::VertexBufferCount);
  bool positions_ok = CreateVertexBuffer(mesh.Positions, device, gpu_mesh->VertexBuffers[GpuMesh::PositionVertexBufferIndex].GetAddressOf());
  if (!positions_ok) {
    return false;
  }

  bool normals_ok = CreateVertexBuffer(mesh.Normals, device, gpu_mesh->VertexBuffers[GpuMesh::NormalVertexBufferIndex].GetAddressOf());
  if (!normals_ok) {
    return false;
  }

  bool texture_coords_ok = CreateVertexBuffer(mesh.TextureCoords, device, gpu_mesh->VertexBuffers[GpuMesh::TextureCoordVertexBufferIndex].GetAddressOf());
  if (!texture_coords_ok) {
    return false;
  }

  bool indices_ok = CreateIndexBuffer(mesh.Indices, device, gpu_mesh->VertexBuffers[GpuMesh::TextureCoordVertexBufferIndex].GetAddressOf());
  if (!indices_ok) {
    return false;
  }

  return true;
}

std::array<D3D11_INPUT_ELEMENT_DESC, GpuMesh::MeshLayoutElementCount> GpuMesh::InitializeLayout() {
  std::array<D3D11_INPUT_ELEMENT_DESC, GpuMesh::MeshLayoutElementCount> result;
  size_t insert_pos = 0;

  for (size_t i = 0; i < GpuMesh::PositionType::InputLayoutElementCount; ++i) {
    result[insert_pos] = GpuMesh::PositionType::InputLayout[i];
    ++insert_pos;
  }

  for (size_t i = 0; i < GpuMesh::NormalType::InputLayoutElementCount; ++i) {
    result[insert_pos] = GpuMesh::NormalType::InputLayout[i];
    ++insert_pos;
  }

  for (size_t i = 0; i < GpuMesh::TextureType::InputLayoutElementCount; ++i) {
    result[insert_pos] = GpuMesh::TextureType::InputLayout[i];
    ++insert_pos;
  }

  return result;
}
