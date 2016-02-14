#ifndef ELGFORWARD_INTERLEAVEDMESH_H_
#define ELGFORWARD_INTERLEAVEDMESH_H_

#include <vector>

template<typename VT>
class InterleavedMesh {
public:
  using VertexType = VT;

  InterleavedMesh() = default;
  ~InterleavedMesh() = default;

  InterleavedMesh(const InterleavedMesh& other) = delete;
  InterleavedMesh& operator=(const InterleavedMesh& other) = delete;

  InterleavedMesh(InterleavedMesh&& other) = default;
  InterleavedMesh& operator=(InterleavedMesh&& other) = default;

  template<class... Args>
  void AddVertex(Args&&... args) {
    m_vertices_.emplace_back(std::forward<Args>(args)...)
  }
  
  void AddIndex(uint32_t index) {
    m_indices_.emplace_back(index);
  }

private:
  std::vector<VT> m_vertices_;
  std::vector<uint32_t> m_indices_;
};

#endif  // ELGFORWARD_INTERLEAVEDMESH_H_
