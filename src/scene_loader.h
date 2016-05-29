#pragma once

#include "scene.h"

bool LoadScene(const filesystem::path& path, const filesystem::path& base_path, ID3D11Device* device, Scene* scene);
