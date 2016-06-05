#pragma once

#include "scene.h"
#include "directx_state.h"

bool LoadScene(const filesystem::path& path, const filesystem::path& base_path, DirectXState* state, Scene* scene);
