#include "json_helpers.h"

#include <fstream>

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "filesystem.h"

bool ReadJsonFile(const filesystem::path& path, nlohmann::json* json) {
  std::ifstream input(path);

  if (!input.good()) {
    return false;
  }

  input >> *json;

  return true;
}

bool ReadFloat(const nlohmann::json& json_value, float* output) {
  if (json_value.is_number_float()) {
    *output = json_value;
    return true;
  }

  if (!json_value.is_array() || json_value.size() != 1) {
    return false;
  }

  if (!json_value[0].is_number_float()) {
    return false;
  }

  *output = json_value[0];
  return true;
}

bool ReadFloat2(const nlohmann::json& json_value, float* output) {
  if (!json_value.is_array() || json_value.size() != 2) {
    return false;
  }

  if (!json_value[0].is_number_float()) {
    return false;
  }

  if (!json_value[1].is_number_float()) {
    return false;
  }

  output[0] = json_value[0];
  output[1] = json_value[1];
  return true;
}

bool ReadFloat3(const nlohmann::json& json_value, float* output) {
  if (!json_value.is_array() || json_value.size() != 3) {
    return false;
  }

  if (!json_value[0].is_number_float()) {
    return false;
  }

  if (!json_value[1].is_number_float()) {
    return false;
  }

  if (!json_value[2].is_number_float()) {
    return false;
  }

  output[0] = json_value[0];
  output[1] = json_value[1];
  output[2] = json_value[2];
  return true;
}

bool ReadFloat4(const nlohmann::json& json_value, float* output) {
  if (!json_value.is_array() || json_value.size() != 4) {
    return false;
  }

  if (!json_value[0].is_number_float()) {
    return false;
  }

  if (!json_value[1].is_number_float()) {
    return false;
  }

  if (!json_value[2].is_number_float()) {
    return false;
  }

  if (!json_value[3].is_number_float()) {
    return false;
  }

  output[0] = json_value[0];
  output[1] = json_value[1];
  output[2] = json_value[2];
  output[3] = json_value[3];
  return true;
}

bool ReadBool(const nlohmann::json& json_value, bool* value) {
  if (!json_value.is_boolean()) {
    return false;
  }

  *value = json_value;
  return true;
}
