#pragma once

#pragma warning(push)
#pragma warning(disable: 4706)
#include <nlohmann/json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "rendering/lens/perspective_lens.h"
#include "rendering/cameras/trackball_camera.h"
#include "rendering/camera_script.h"
#include "directx_state.h"

namespace Loaders {

bool ReadCamera(const nlohmann::json& json_camera, const filesystem::path& base_path, DirectXState* state,
                Rendering::Cameras::TrackballCamera* camera, Rendering::Lens::PerspectiveLens* lens,
                Rendering::CameraScript* script);

}  // namespace Loaders
