#include "chaiscript_helpers.h"

#include <DirectXMath.h>

#include "dxfw/dxfw_helpers.h"

namespace Core {

void PrepareChaiscript(chaiscript::ChaiScript* script) {
  // DirectXMath
  script->add(chaiscript::fun(static_cast<float(*)(float)>(&DirectX::XMScalarSin)), "sin");
  script->add(chaiscript::fun(static_cast<float(*)(float)>(&DirectX::XMScalarCos)), "cos");

  // Tracing
  script->add(chaiscript::fun([](const std::string& msg) {
    DXFW_TRACE(__FILE__, __LINE__, false, "%S", msg.c_str());
  }), "dxfw_trace");
}

}  // namespace Core
