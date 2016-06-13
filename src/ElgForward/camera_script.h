#pragma once


#include "filesystem.h"
#include "chaiscript_helpers.h"
#include "perspective_lens.h"
#include "trackball_camera.h"

class CameraScript {
 public:
   bool init(const filesystem::path& path, TrackballCamera* camera, PerspectiveLens* lens) {
     try {
       // Prepare
       PrepareChaiscript(&m_script_);

       // Setup camera
       m_script_.add(chaiscript::user_type<TrackballCamera>(), "TrackballCamera");
       m_script_.add(chaiscript::fun(&TrackballCamera::GetLocation), "GetLocation");
       m_script_.add(chaiscript::fun(&TrackballCamera::SetLocation), "SetLocation");
       m_script_.add(chaiscript::fun(&TrackballCamera::GetRadius), "GetRadius");
       m_script_.add(chaiscript::fun(&TrackballCamera::SetRadius), "SetRadius");
       m_script_.add(chaiscript::fun(&TrackballCamera::LookAt), "LookAt");

       m_script_.add_global(chaiscript::var(std::ref(*camera)), "camera");

       // Setup lens
       m_script_.add(chaiscript::user_type<PerspectiveLens>(), "PerspectiveLens");
       m_script_.add(chaiscript::fun(&PerspectiveLens::GetZoomFactor), "GetZoomFactor");
       m_script_.add(chaiscript::fun(&PerspectiveLens::SetZoomFactor), "SetZoomFactor");

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
  chaiscript::ChaiScript m_script_ = { chaiscript::Std_Lib::library() };

  std::function<void(float)> m_update_fun_ = {};
};