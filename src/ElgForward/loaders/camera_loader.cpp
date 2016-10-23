#include "camera_loader.h"

#pragma warning(push)
#pragma warning(disable: 4706)
#include <json.hpp>
#pragma warning(pop)

#include "core/filesystem.h"
#include "rendering/camera_script.h"
#include "rendering/screen.h"
#include "rendering/cameras/trackball_camera.h"
#include "rendering/lens/perspective_lens.h"
#include "directx_state.h"

using namespace Rendering::Cameras;
using namespace Rendering::Lens;
using namespace Rendering;

namespace Loaders {

void ConnectCameraToInput(DirectXState* state, TrackballCamera* camera, PerspectiveLens* lens) {
  Dxfw::RegisterMouseButtonCallback(state->window.get(), [camera, viewport = &state->viewport](dxfwWindow*, dxfwMouseButton button, dxfwMouseButtonAction action, int16_t x, int16_t y) {
    if (button == DXFW_RIGHT_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_DOWN) {
      camera->SetDesiredState(TrackballCameraOperation::Panning);
      camera->SetEndPoint(GetNormalizedScreenCoordinates(viewport->Width, viewport->Height, x, y));
    } else if (button == DXFW_RIGHT_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_UP) {
      camera->SetDesiredState(TrackballCameraOperation::None);
    } else if (button == DXFW_LEFT_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_DOWN) {
      camera->SetDesiredState(TrackballCameraOperation::Rotating);
      camera->SetEndPoint(GetNormalizedScreenCoordinates(viewport->Width, viewport->Height, x, y));
    } else if (button == DXFW_LEFT_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_UP) {
      camera->SetDesiredState(TrackballCameraOperation::None);
    } else if (button == DXFW_MIDDLE_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_DOWN) {
      camera->SetDesiredState(TrackballCameraOperation::Zooming);
      camera->SetEndPoint(GetNormalizedScreenCoordinates(viewport->Width, viewport->Height, x, y));
    } else if (button == DXFW_MIDDLE_MOUSE_BUTTON && action == DXFW_MOUSE_BUTTON_UP) {
      camera->SetDesiredState(TrackballCameraOperation::None);
    }
  });

  Dxfw::RegisterMouseMoveCallback(state->window.get(), [camera, viewport = &state->viewport](dxfwWindow*, int16_t x, int16_t y) {
    camera->SetEndPoint(GetNormalizedScreenCoordinates(viewport->Width, viewport->Height, x, y));
  });

  Dxfw::RegisterMouseWheelCallback(state->window.get(), [lens](dxfwWindow*, int16_t, int16_t, int16_t delta) {
    if (delta > 0) {
      lens->SetZoomFactor(1.1f * lens->GetZoomFactor());
    } else {
      lens->SetZoomFactor(0.9f * lens->GetZoomFactor());
    }
  });
}

bool ReadCamera(const nlohmann::json& json_camera, const filesystem::path& base_path, DirectXState* state, TrackballCamera* camera, PerspectiveLens* lens, CameraScript* script) {
  const auto& json_lens = json_camera["prespective_lens"];
  if (json_lens.is_object()) {
    const auto& near_plane = json_lens["near_plane"];
    const auto& far_plane = json_lens["far_plane"];
    const auto& fov = json_lens["fov"];

    bool is_valid_lens_entry = near_plane.is_number_float()
      && far_plane.is_number_float()
      && fov.is_number_float();
    if (is_valid_lens_entry) {
      lens->SetFrustum(near_plane, far_plane, state->viewport.Width / state->viewport.Height, DirectX::XMConvertToRadians(fov));
    }
  }

  const auto& json_trackball_camera = json_camera["trackball_camera"];
  if (json_trackball_camera.is_object()) {
    const auto& radius = json_trackball_camera["radius"];
    if (radius.is_number_float()) {
      camera->SetRadius(radius);
    }

    const auto& json_position = json_trackball_camera["position"];
    if (json_position.is_array() && json_position.size() == 3) {
      const auto& x = json_position[0];
      const auto& y = json_position[1];
      const auto& z = json_position[2];
      if (x.is_number_float() && y.is_number_float() && z.is_number_float()) {
        camera->SetLocation(x, y, z);
      }
    }
  }

  const auto& json_camera_script = json_camera["script"];
  if (json_camera_script.is_string()) {
    auto path = base_path / json_camera_script;
    bool init_ok = script->init(path, camera, lens);
    if (!init_ok) {
      DXFW_TRACE(__FILE__, __LINE__, false, "Error initializing camera script from file %S", path.string());
    }
  }

  ConnectCameraToInput(state, camera, lens);

  return true;
}

}  // namespace Loaders
