#pragma once

#pragma warning(push)
#pragma warning(disable: 4602)
#pragma warning(disable: 4244)
#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>
#pragma warning(pop)

namespace Core {

void PrepareChaiscript(chaiscript::ChaiScript* script);

}  // namespace Core
