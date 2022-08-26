#pragma once
#include <vector>
#include <Windows.h>
#include "d3dDefine.h"

struct tWindowSwap {
	HWND m_hWnd;
	UINT m_SwapBufferCount;

	ComPtr<IDXGISwapChain> m_pSwapChain;          // IDXGISwapChain.size == window.size
	ComPtr<ID3D11Texture2D> m_pSwapBackTexture2D; // back buffer of swapchain
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;

public:
	bool InitSwap(ComPtr<IDXGIFactory1> pFactory1, ComPtr<ID3D11Device> pDevice, HWND hWnd);
	bool TestResizeSwapChain(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pDeviceContext);

private:
	bool _CreateTargetView(ComPtr<ID3D11Device> pDevice);
};
