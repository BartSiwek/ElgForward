#include "drawable.h"

bool CreateDrawable(const GpuMesh& /* mesh */, const Material& /* material */, ID3D11Device* /* device */) {
  // Foreach input to shader
  //   Get the input name and index
  //   Map (name, index) -> VertexDataChannel
  //   Use VertexDataChannel to get the vertex buffer from mesh
  //   Add vertex buffer to Drawable VertexBuffers at current loop index
  //   
  //   Create D3D11_INPUT_ELEMENT_DESC using:
  //     SemanticName - from shader
  //     SemanticIndex - from shader
  //     Format - from shader (verify that it matches the one in mesh)
  //     InputSlot - the loop index
  //     AlignedByteOffset - D3D11_APPEND_ALIGNED_ELEMENT
  //     InputSlotClass - D3D11_INPUT_PER_VERTEX_DATA
  //     InstanceDataStepRate - 0

  // Create the ID3D11InputLayout based on the previous and add it to drawable

  // TODO: Map (name, index) -> VertexDataChannel 
  // TODO: Use VertexDataChannel to get the vertex buffer from mesh
  // TODO: Verify shader format matches mesh format

  return true;
}