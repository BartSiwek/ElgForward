#include "filesystem.h"

#include <Shlwapi.h>

filesystem::path GetBasePath() {
  wchar_t path[MAX_PATH];
  GetModuleFileNameW(nullptr, path, MAX_PATH);
  PathRemoveFileSpecW(path);
  return filesystem::path(path);
}
