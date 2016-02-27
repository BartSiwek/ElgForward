#pragma once

#ifndef SAFE_RELEASE

#define SAFE_RELEASE(x)  \
   if(x != nullptr)      \
   {                     \
      x->Release();      \
      x = nullptr;       \
   }

#endif  // SAFE_RELEASE


