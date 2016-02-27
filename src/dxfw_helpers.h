#pragma once

#include <dxfw/dxfw.h>

class DxfwGuard {
public:
  DxfwGuard() : m_is_initialized_(false) {
    m_is_initialized_ = dxfwInitialize();
  }

  DxfwGuard(const DxfwGuard&) = delete;
  DxfwGuard& operator=(const DxfwGuard&) = delete;
  DxfwGuard(DxfwGuard&&) = delete;
  DxfwGuard& operator=(DxfwGuard&&) = delete;

  ~DxfwGuard() {
    if (m_is_initialized_) {
      dxfwTerminate();
    }
  }

  bool IsInitialized() {
    return m_is_initialized_;
  }

private:
  bool m_is_initialized_;
};

class DxfwWindowDeleter {
public:
  void operator()(dxfwWindow* window) const {
    dxfwDestroyWindow(window);
  }
};

