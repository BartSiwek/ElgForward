#pragma once

#include "scene.h"
#include "directx_state.h"

namespace Loaders {

bool LoadScene(const filesystem::path& path, const filesystem::path& base_path, DirectXState* state, Scene* scene);

}  // namespace Loaders
