#include "mesh_loader.h"

#include <fstream>
#include <memory>

#include <tiny_obj_loader.h>

#include <dxfw/dxfw.h>

#include "filesystem.h"

class NullMaterialReader : public tinyobj::MaterialReader {
public:
  NullMaterialReader() {}
  
  virtual ~NullMaterialReader() {}

  virtual bool operator()(const std::string & /* matId */, std::vector<tinyobj::material_t> & /* materials */,
                          std::map<std::string, int> & /* matMap */, std::string & /* err */) {
    return true;
  }
};

static NullMaterialReader g_null_material_reader_intance_;

bool LoadMesh(const filesystem::path& path, std::vector<Mesh>* meshes) {
  if (meshes == nullptr) {
    DXFW_TRACE(__FILE__, __LINE__, "Got a null mesh vector pointer", true);
    return false;
  }

  std::ifstream input_stream(path.wstring());
  if (!input_stream) {
    DXFW_TRACE(__FILE__, __LINE__, "Cannot open mesh file", true);
    return false;
  }

  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;
  bool ret = tinyobj::LoadObj(shapes, materials, err, input_stream, g_null_material_reader_intance_);

  if (!err.empty()) {
    DXFW_TRACE(__FILE__, __LINE__, err.c_str(), true);
  }

  if (!ret) {
    return false;
  }

  meshes->reserve(shapes.size());
  for (const auto& shape : shapes) {
    auto vertex_count = shape.mesh.positions.size() / 3;
    auto index_count = shape.mesh.positions.size();

    meshes->emplace_back(vertex_count, index_count);
    auto& mesh = meshes->back();

    for (size_t v = 0; v < 3 * vertex_count; v += 3) {
      DirectX::XMFLOAT3 position = { shape.mesh.positions[v], shape.mesh.positions[v + 1], shape.mesh.positions[v + 2] };
      DirectX::XMFLOAT3 normal = {shape.mesh.normals[v], shape.mesh.normals[v + 1], shape.mesh.normals[v + 2] };
      DirectX::XMFLOAT2 tex_coord = { shape.mesh.texcoords[v], shape.mesh.texcoords[v + 1] };

      mesh.AddVertex(position, normal, tex_coord);
    }

    for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
      mesh.AddIndex(shape.mesh.indices[i]);
    }
  }

  return true;

  /*
  std::cout << "# of shapes    : " << shapes.size() << std::endl;
  std::cout << "# of materials : " << materials.size() << std::endl;

  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", i, shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %ld\n", i, shapes[i].mesh.indices.size());
    printf("Size of shape[%ld].material_ids: %ld\n", i, shapes[i].mesh.material_ids.size());
    assert((shapes[i].mesh.indices.size() % 3) == 0);
    for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
      unsigned int a = shapes[i].mesh.indices[3 * f + 0];
      unsigned int b = shapes[i].mesh.indices[3 * f + 1];
      unsigned int c = shapes[i].mesh.indices[3 * f + 2];

      printf("  idx[%ld] = %d, %d, %d. mat_id = %d\n", f, a, b, c, shapes[i].mesh.material_ids[f]);
      printf("  (%f, %f, %f) -> (%f, %f, %f) -> (%f, %f, %f)\n",
        shapes[i].mesh.positions[3 * a + 0], shapes[i].mesh.positions[3 * a + 1], shapes[i].mesh.positions[3 * a + 2],
        shapes[i].mesh.positions[3 * b + 0], shapes[i].mesh.positions[3 * b + 1], shapes[i].mesh.positions[3 * b + 2],
        shapes[i].mesh.positions[3 * c + 0], shapes[i].mesh.positions[3 * c + 1], shapes[i].mesh.positions[3 * c + 2]);
    }

    printf("shape[%ld].vertices: %ld\n", i, shapes[i].mesh.positions.size());
    assert((shapes[i].mesh.positions.size() % 3) == 0);
    for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
      printf("  v[%ld] = (%f, %f, %f)\n", v,
        shapes[i].mesh.positions[3 * v + 0],
        shapes[i].mesh.positions[3 * v + 1],
        shapes[i].mesh.positions[3 * v + 2]);
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", i, materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n", materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
    printf("  material.Kd = (%f, %f ,%f)\n", materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
    printf("  material.Ks = (%f, %f ,%f)\n", materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
    printf("  material.Tr = (%f, %f ,%f)\n", materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]);
    printf("  material.Ke = (%f, %f ,%f)\n", materials[i].emission[0], materials[i].emission[1], materials[i].emission[2]);
    printf("  material.Ns = %f\n", materials[i].shininess);
    printf("  material.Ni = %f\n", materials[i].ior);
    printf("  material.dissolve = %f\n", materials[i].dissolve);
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n", materials[i].specular_highlight_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(materials[i].unknown_parameter.end());
    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
  */
}
