#pragma once

#include <vector>

#include <d3d11.h>

#include "perspective_lens.h"
#include "trackball_camera.h"
#include "drawable.h"

struct Scene {
  std::vector<Drawable> drawables;
  PerspectiveLens lens;
  TrackballCamera camera;
};
