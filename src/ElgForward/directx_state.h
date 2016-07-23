#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "dxfw/dxfw_helpers.h"

struct DirectXState {
  Dxfw::DxfwWindowUniquePtr window;
  Microsoft::WRL::ComPtr<ID3D11Device> device;
  Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
  D3D11_VIEWPORT viewport;
};
