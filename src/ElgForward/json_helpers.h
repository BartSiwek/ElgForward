#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "filesystem.h"

bool ReadJsonFile(const filesystem::path& path, nlohmann::json* json);

bool ReadFloat(const nlohmann::json& json_value, float* output);

bool ReadFloat2(const nlohmann::json& json_value, float* output);

bool ReadFloat3(const nlohmann::json& json_value, float* output);

bool ReadFloat4(const nlohmann::json& json_value, float* output);

bool ReadBool(const nlohmann::json& json_value, bool* value);
