#pragma once

#include <cstdint>
#include <functional>

#include <windows.h>

#include <dxfw/dxfw.h>

namespace Dxfw {

/* INIT & TERMINATE */
bool Initialize();
void Terminate();

/* WINDOW MANAGEMENT */
dxfwWindow* CreateNewWindow(uint32_t width, uint32_t height, const char* caption);
void DestroyWindow(dxfwWindow* window);

/* WINDOW HANDLE */
HWND GetWindowHandle(dxfwWindow* window);

/* WINDOW STATE */
void GetWindowSize(dxfwWindow* window, uint32_t* width, uint32_t* height);
bool ShouldWindowClose(dxfwWindow* window);

/* EVENT MANEGEMENT */
void PollOsEvents();

/* SIZE CHANGED EVENT */
uint32_t RegisterWindowResizeCallback(dxfwWindow* window, std::function<void(dxfwWindow*, uint32_t, uint32_t)> callback);

/* MOUSE EVENTS */
uint32_t RegisterMouseButtonCallback(dxfwWindow* window, std::function<void(dxfwWindow*, dxfwMouseButton, dxfwMouseButtonAction, int16_t, int16_t)> callback);
uint32_t RegisterMouseMoveCallback(dxfwWindow* window, std::function<void(dxfwWindow*, int16_t, int16_t)> callback);
uint32_t RegisterMouseWheelCallback(dxfwWindow* window, std::function<void(dxfwWindow*, int16_t, int16_t, int16_t)> callback);

}  // Dxfw
