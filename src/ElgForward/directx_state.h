#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "dxfw/dxfw_helpers.h"

struct DirectXState {
  Dxfw::DxfwWindowUniquePtr window;
  Microsoft::WRL::ComPtr<ID3D11Device> device;
  Microsoft::WRL::ComPtr<ID3D11Debug> debug;
  Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_state;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> linear_sampler;
  D3D11_VIEWPORT viewport;
};
