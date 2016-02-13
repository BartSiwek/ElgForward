#include <iostream>
#include <tiny_obj_loader.h>
#include <json.hpp>

int main() {
  std::cout << "Hello forward" << std::endl;

  std::vector<int> c_vector{ 1, 2, 3, 4 };
  nlohmann::json j_vec(c_vector);
  std::cout << j_vec.dump() << std::endl;

  return 0;
}