#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "directx_state.h"
#include "trackball_camera.h"
#include "perspective_lens.h"
#include "camera_script.h"

namespace Loaders {

bool ReadCamera(const nlohmann::json& json_camera, const filesystem::path& base_path, DirectXState* state, TrackballCamera* camera, PerspectiveLens* lens, CameraScript* script);

}  // namespace Loaders
