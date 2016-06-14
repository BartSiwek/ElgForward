#pragma once

#include <vector>

#include <d3d11.h>

#include "perspective_lens.h"
#include "trackball_camera.h"
#include "camera_script.h"
#include "drawable.h"
#include "directional_light.h"
#include "spot_light.h"
#include "point_light.h"

struct Scene {
  std::vector<Drawable> drawables;
  PerspectiveLens lens;
  TrackballCamera camera;
  CameraScript camera_script;
  std::vector<DirectionalLight> directional_lights;
  std::vector<SpotLight> spot_lights;
  std::vector<PointLight> point_lights;
};
