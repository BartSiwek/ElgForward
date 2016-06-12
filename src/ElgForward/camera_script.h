#pragma once

#include <DirectXMath.h>

#pragma warning(push)
#pragma warning(disable: 4602)
#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>
#pragma warning(pop)

#include "filesystem.h"
#include "dxfw_helpers.h"
#include "perspective_lens.h"
#include "trackball_camera.h"

class CameraScript {
 public:
   CameraScript() : m_script_(chaiscript::Std_Lib::library()) {
   }

   bool init(const filesystem::path& path, TrackballCamera* camera, PerspectiveLens* lens) {
     try {
       // Prepare
       prepare(&m_script_);

       // Setup camera
       m_script_.add(chaiscript::user_type<TrackballCamera>(), "TrackballCamera");
       m_script_.add(chaiscript::fun(&TrackballCamera::LookAt), "LookAt");

       m_script_.add_global(chaiscript::var(std::ref(*camera)), "camera");

       // Setup lens
       m_script_.add(chaiscript::user_type<PerspectiveLens>(), "PerspectiveLens");

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
  void prepare(chaiscript::ChaiScript* script) {
    // DirectXMath
    script->add(chaiscript::fun(static_cast<float(*)(float)>(&DirectX::XMScalarSin)), "sin");
    script->add(chaiscript::fun(static_cast<float(*)(float)>(&DirectX::XMScalarCos)), "cos");

    // Tracing
    script->add(chaiscript::fun([](const std::string& msg) {
      DXFW_TRACE(__FILE__, __LINE__, false, "%S", msg.c_str());
    }), "dxfw_trace");

  }

  chaiscript::ChaiScript m_script_;

  std::function<void(float)> m_update_fun_;
};