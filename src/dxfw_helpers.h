#pragma once

#include <memory>

#include "dxfw_wrapper.h"

class DxfwGuard {
public:
  DxfwGuard() {
    if (s_initialization_counter_ == 0) {
      bool init_ok = Dxfw::Initialize();
      if (init_ok) {
        ++s_initialization_counter_;
      }
    } else {
      ++s_initialization_counter_;
    }
  }

  DxfwGuard(const DxfwGuard&) = delete;
  DxfwGuard& operator=(const DxfwGuard&) = delete;
  DxfwGuard(DxfwGuard&&) = delete;
  DxfwGuard& operator=(DxfwGuard&&) = delete;

  ~DxfwGuard() {
    if (s_initialization_counter_ > 0) {
      --s_initialization_counter_;
      if (s_initialization_counter_ == 0) {
        Dxfw::Terminate();
      }
    }
  }

  bool IsInitialized() {
    return (s_initialization_counter_ != 0);
  }

private:
  static uint32_t s_initialization_counter_;
};

class DxfwWindowDeleter {
public:
  void operator()(dxfwWindow* window) const {
    Dxfw::DestroyWindow(window);
  }
};

using dxfwWindowUniquePtr = std::unique_ptr<dxfwWindow, DxfwWindowDeleter>;
