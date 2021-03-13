#pragma once


#include "core/filesystem.h"
#include "core/chaiscript_helpers.h"
#include "rendering/lens/perspective_lens.h"
#include "rendering/cameras/trackball_camera.h"

namespace Rendering {

class CameraScript {
 public:
   bool init(const filesystem::path& path, Cameras::TrackballCamera* camera, Lens::PerspectiveLens* lens) {
     try {
       // Prepare
       Core::PrepareChaiscript(&m_script_);

       // Setup camera
       m_script_.add(chaiscript::user_type<Cameras::TrackballCamera>(), "TrackballCamera");
       m_script_.add(chaiscript::fun(&Cameras::TrackballCamera::GetLocation), "GetLocation");
       m_script_.add(chaiscript::fun(&Cameras::TrackballCamera::SetLocation), "SetLocation");
       m_script_.add(chaiscript::fun(&Cameras::TrackballCamera::GetRadius), "GetRadius");
       m_script_.add(chaiscript::fun(&Cameras::TrackballCamera::SetRadius), "SetRadius");
       m_script_.add(chaiscript::fun(&Cameras::TrackballCamera::LookAt), "LookAt");

       m_script_.add_global(chaiscript::var(std::ref(*camera)), "camera");

       // Setup lens
       m_script_.add(chaiscript::user_type<Lens::PerspectiveLens>(), "PerspectiveLens");
       m_script_.add(chaiscript::fun(&Lens::PerspectiveLens::GetZoomFactor), "GetZoomFactor");
       m_script_.add(chaiscript::fun(&Lens::PerspectiveLens::SetZoomFactor), "SetZoomFactor");

       m_script_.add_global(chaiscript::var(std::ref(*lens)), "lens");

       // Load
       m_script_.use(path.string());

       // Setup update
       m_update_fun_ = m_script_.eval<std::function<void(float)>>("update");
       if (!m_update_fun_) {
         return false;
       }

       return true;
     } catch (const chaiscript::exception::file_not_found_error& ex) {
       DXFW_TRACE(__FILE__, __LINE__, false, "Script not found: %S", ex.what());
       return false;
     } catch (const chaiscript::exception::eval_error& ex) {
       DXFW_TRACE(__FILE__, __LINE__, false, "Evaluation error: %S", ex.what());
       return false;
     }
   }

   void update(float t) {
     if (m_update_fun_) {
       m_update_fun_(t);
     }
   }

 private:
  chaiscript::ChaiScript m_script_ = {};

  std::function<void(float)> m_update_fun_ = {};
};

}  // namespace Rendering
