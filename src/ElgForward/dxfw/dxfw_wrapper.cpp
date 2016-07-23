#include "dxfw_wrapper.h"

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

#include <dxfw/dxfw.h>

namespace Dxfw {

/* Structs */
class ResizeCallback {
public:
  static const uint32_t INVALID_CALLBACK_KEY = static_cast<uint32_t>(-1);

  using CallbackFunction = std::function<void(dxfwWindow*, uint32_t, uint32_t)>;

  ResizeCallback(uint32_t key, CallbackFunction&& callback) : m_key_(key), m_callback_(callback) {
  }

  ResizeCallback(const ResizeCallback&) = delete;
  ResizeCallback& operator=(const ResizeCallback&) = delete;

  ResizeCallback(ResizeCallback&&) = default;
  ResizeCallback& operator=(ResizeCallback&&) = default;

  ~ResizeCallback() {
  }

  uint32_t GetKey() const {
    return m_key_;
  }

  void operator()(dxfwWindow* window, uint32_t width, uint32_t height) const {
    m_callback_(window, width, height);
  }

private:
  const uint32_t m_key_;
  const CallbackFunction m_callback_;
};

class MouseButtonCallback {
public:
  static const uint32_t INVALID_CALLBACK_KEY = static_cast<uint32_t>(-1);

  using CallbackFunction = std::function<void(dxfwWindow*, dxfwMouseButton, dxfwMouseButtonAction, int16_t, int16_t)>;

  MouseButtonCallback(uint32_t key, CallbackFunction&& callback) : m_key_(key), m_callback_(callback) {
  }

  MouseButtonCallback(const MouseButtonCallback&) = delete;
  MouseButtonCallback& operator=(const MouseButtonCallback&) = delete;

  MouseButtonCallback(MouseButtonCallback&&) = default;
  MouseButtonCallback& operator=(MouseButtonCallback&&) = default;

  ~MouseButtonCallback() {
  }

  uint32_t GetKey() const {
    return m_key_;
  }

  void operator()(dxfwWindow* window, dxfwMouseButton button, dxfwMouseButtonAction action, int16_t x, int16_t y) const {
    m_callback_(window, button, action, x, y);
  }

private:
  const uint32_t m_key_;
  const CallbackFunction m_callback_;
};

class MouseMoveCallback {
public:
  static const uint32_t INVALID_CALLBACK_KEY = static_cast<uint32_t>(-1);

  using CallbackFunction = std::function<void(dxfwWindow*, int16_t, int16_t)>;

  MouseMoveCallback(uint32_t key, CallbackFunction&& callback) : m_key_(key), m_callback_(callback) {
  }

  MouseMoveCallback(const MouseMoveCallback&) = delete;
  MouseMoveCallback& operator=(const MouseMoveCallback&) = delete;

  MouseMoveCallback(MouseMoveCallback&&) = default;
  MouseMoveCallback& operator=(MouseMoveCallback&&) = default;

  ~MouseMoveCallback() {
  }

  uint32_t GetKey() const {
    return m_key_;
  }

  void operator()(dxfwWindow* window, int16_t x, int16_t y) const {
    m_callback_(window, x, y);
  }

private:
  const uint32_t m_key_;
  const CallbackFunction m_callback_;
};

class MouseWheelCallback {
public:
  static const uint32_t INVALID_CALLBACK_KEY = static_cast<uint32_t>(-1);

  using CallbackFunction = std::function<void(dxfwWindow*, int16_t, int16_t, int16_t)>;

  MouseWheelCallback(uint32_t key, CallbackFunction&& callback) : m_key_(key), m_callback_(callback) {
  }

  MouseWheelCallback(const MouseMoveCallback&) = delete;
  MouseWheelCallback& operator=(const MouseWheelCallback&) = delete;

  MouseWheelCallback(MouseWheelCallback&&) = default;
  MouseWheelCallback& operator=(MouseWheelCallback&&) = default;

  ~MouseWheelCallback() {
  }

  uint32_t GetKey() const {
    return m_key_;
  }

  void operator()(dxfwWindow* window, int16_t x, int16_t y, int16_t delta) const {
    m_callback_(window, x, y, delta);
  }

private:
  const uint32_t m_key_;
  const CallbackFunction m_callback_;
};

struct WindowData {
  std::vector<ResizeCallback> ResizeCallbacks = {};
  std::vector<MouseButtonCallback> MouseButtonCallbacks = {};
  std::vector<MouseMoveCallback> MouseMoveCallbacks = {};
  std::vector<MouseWheelCallback> MouseWheelCallbacks = {};
};

class WrapperData {
public:
  WrapperData() = default;

  void SetInitialized() {
    m_initialized_ = true;
  }

  bool IsInitialized() {
    return m_initialized_;
  }

  uint32_t GenerateNewResizeCallbackKey() {
    return m_last_resize_callback_key_++;
  }

  uint32_t GenerateNewMouseButtonCallbackKey() {
    return m_last_mouse_button_callback_key_++;
  }

  uint32_t GenerateNewMouseMoveCallbackKey() {
    return m_last_mouse_move_callback_key_++;
  }

  uint32_t GenerateNewMouseWheelCallbackKey() {
    return m_last_mouse_wheel_callback_key_++;
  }

  WindowData* AllocateWindowData() {
    m_window_data_.emplace_back(std::make_unique<WindowData>());
    return m_window_data_.back().get();
  }

  void DeallocateWindowData(WindowData* data) {
    auto it = std::find_if(std::begin(m_window_data_), std::end(m_window_data_), [data](const std::unique_ptr<WindowData>& value) {
      return value.get() == data;
    });
    if (it != std::end(m_window_data_)) {
      it->reset();
      m_window_data_.erase(it);
    }
  }

  void Reset() {
    m_initialized_ = false;
    m_last_resize_callback_key_ = 0;
    m_last_mouse_button_callback_key_ = 0;
    m_last_mouse_move_callback_key_ = 0;
    m_last_mouse_wheel_callback_key_ = 0;
    m_window_data_.clear();
  }

private:
  bool m_initialized_ = false;
  uint32_t m_last_resize_callback_key_ = 0;
  uint32_t m_last_mouse_button_callback_key_ = 0;
  uint32_t m_last_mouse_move_callback_key_ = 0;
  uint32_t m_last_mouse_wheel_callback_key_ = 0;
  std::vector<std::unique_ptr<WindowData>> m_window_data_ = {};
};

/* Globals */
WrapperData g_wrapper_data_;

/* Helpers */

/* DXFW Callbacks */
void DxfwErrorCallback(dxfwError error) {
  DXFW_ERROR_TRACE(__FILE__, __LINE__, true, error);
}

void DxfwResizeCallback(dxfwWindow* window, uint32_t width, uint32_t height) {
  WindowData* data = static_cast<WindowData*>(dxfwGetWindowUserData(window));
  if (data != nullptr) {
    for (const auto& callback : data->ResizeCallbacks) {
      callback(window, width, height);
    }
  }
}

void DxfwMouseButtonCallback(dxfwWindow* window, dxfwMouseButton button, dxfwMouseButtonAction action, int16_t x, int16_t y) {
  WindowData* data = static_cast<WindowData*>(dxfwGetWindowUserData(window));
  if (data != nullptr) {
    for (const auto& callback : data->MouseButtonCallbacks) {
      callback(window, button, action, x, y);
    }
  }
}

void DxfwMouseMoveCallback(dxfwWindow* window, int16_t x, int16_t y) {
  WindowData* data = static_cast<WindowData*>(dxfwGetWindowUserData(window));
  if (data != nullptr) {
    for (const auto& callback : data->MouseMoveCallbacks) {
      callback(window, x, y);
    }
  }
}

void DxfwMouseWheelCallback(dxfwWindow* window, int16_t x, int16_t y, int16_t delta) {
  WindowData* data = static_cast<WindowData*>(dxfwGetWindowUserData(window));
  if (data != nullptr) {
    for (const auto& callback : data->MouseWheelCallbacks) {
      callback(window, x, y, delta);
    }
  }
}

/* DXFW Wrapper functions */
bool Initialize() {
  if (!g_wrapper_data_.IsInitialized()) {
    bool init_ok = dxfwInitialize();
    if (init_ok) {
      dxfwSetErrorCallback(DxfwErrorCallback);

      g_wrapper_data_.SetInitialized();

      return true;
    }
    return false;
  }
  return false;
}

void Terminate() {
  if (g_wrapper_data_.IsInitialized()) {
    g_wrapper_data_.Reset();
    dxfwSetErrorCallback(nullptr);
    dxfwTerminate();
  }
}

dxfwWindow* CreateNewWindow(uint32_t width, uint32_t height, const char* caption) {
  dxfwWindow* window = dxfwCreateWindow(width, height, caption);
  if (window != nullptr) {
    WindowData* data = g_wrapper_data_.AllocateWindowData();
    dxfwSetWindowUserData(window, data);

    dxfwSetSizeChangedCallback(window, DxfwResizeCallback);
    dxfwSetMouseButtonCallback(window, DxfwMouseButtonCallback);
    dxfwSetMouseMoveCallback(window, DxfwMouseMoveCallback);
    dxfwSetMouseWheelCallback(window, DxfwMouseWheelCallback);
  }

  return window;
}

void DestroyWindow(dxfwWindow* window) {
  WindowData* data = static_cast<WindowData*>(dxfwGetWindowUserData(window));
  if (data != nullptr) {
    dxfwSetWindowUserData(window, nullptr);
    g_wrapper_data_.DeallocateWindowData(data);
  }

  dxfwDestroyWindow(window);
}

HWND GetWindowHandle(dxfwWindow* window) {
  return dxfwGetHandle(window);
}

void GetWindowSize(dxfwWindow* window, uint32_t* width, uint32_t* height) {
  dxfwGetWindowSize(window, width, height);
}

void PollOsEvents() {
  dxfwPollOsEvents();
}

bool ShouldWindowClose(dxfwWindow* window) {
  return dxfwShouldWindowClose(window);
}

uint32_t RegisterWindowResizeCallback(dxfwWindow* window, std::function<void(dxfwWindow*, uint32_t, uint32_t)> callback) {
  WindowData* data = static_cast<WindowData*>(dxfwGetWindowUserData(window));
  if (data != nullptr) {
    uint32_t new_key = g_wrapper_data_.GenerateNewResizeCallbackKey();
    data->ResizeCallbacks.emplace_back(new_key, std::move(callback));

    return new_key;
  }
  return ResizeCallback::INVALID_CALLBACK_KEY;
}

uint32_t RegisterMouseButtonCallback(dxfwWindow* window, std::function<void(dxfwWindow*, dxfwMouseButton, dxfwMouseButtonAction, int16_t, int16_t)> callback) {
  WindowData* data = static_cast<WindowData*>(dxfwGetWindowUserData(window));
  if (data != nullptr) {
    uint32_t new_key = g_wrapper_data_.GenerateNewMouseButtonCallbackKey();
    data->MouseButtonCallbacks.emplace_back(new_key, std::move(callback));

    return new_key;
  }
  return MouseButtonCallback::INVALID_CALLBACK_KEY;
}

uint32_t RegisterMouseMoveCallback(dxfwWindow* window, std::function<void(dxfwWindow*, int16_t, int16_t)> callback) {
  WindowData* data = static_cast<WindowData*>(dxfwGetWindowUserData(window));
  if (data != nullptr) {
    uint32_t new_key = g_wrapper_data_.GenerateNewMouseMoveCallbackKey();
    data->MouseMoveCallbacks.emplace_back(new_key, std::move(callback));

    return new_key;
  }
  return MouseMoveCallback::INVALID_CALLBACK_KEY;
}

uint32_t RegisterMouseWheelCallback(dxfwWindow* window, std::function<void(dxfwWindow*, int16_t, int16_t, int16_t)> callback) {
  WindowData* data = static_cast<WindowData*>(dxfwGetWindowUserData(window));
  if (data != nullptr) {
    uint32_t new_key = g_wrapper_data_.GenerateNewMouseWheelCallbackKey();
    data->MouseWheelCallbacks.emplace_back(new_key, std::move(callback));

    return new_key;
  }
  return MouseWheelCallback::INVALID_CALLBACK_KEY;
}

}  // namespace Dxfw
