#include "stdafx.h"
#include "ImplSwap.h"

#define SWAP_CHAIN_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM

bool tWindowSwap::InitSwap(ComPtr<IDXGIFactory1> pFactory1, ComPtr<ID3D11Device> pDevice, HWND hWnd)
{
	m_hWnd = hWnd;
	m_SwapBufferCount = 1;

	CRect rcClient;
	::GetClientRect(hWnd, &rcClient);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferDesc.Width = rcClient.Width();
	sd.BufferDesc.Height = rcClient.Height();
	sd.BufferDesc.Format = SWAP_CHAIN_FORMAT;
	sd.SampleDesc.Count = m_SwapBufferCount;
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;

	HRESULT hr = pFactory1->CreateSwapChain(pDevice, &sd, m_pSwapChain.Assign());
	if (FAILED(hr))
		return false;

	return _CreateTargetView(pDevice);
}

bool tWindowSwap::_CreateTargetView(ComPtr<ID3D11Device> pDevice)
{
	HRESULT hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(m_pSwapBackTexture2D.Assign()));
	if (FAILED(hr))
		return false;

	hr = pDevice->CreateRenderTargetView(m_pSwapBackTexture2D, nullptr, m_pRenderTargetView.Assign());
	if (FAILED(hr))
		return false;

	return true;
}

bool tWindowSwap::TestResizeSwapChain(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pDeviceContext)
{
	CRect rcClient;
	::GetClientRect(m_hWnd, &rcClient);

	D3D11_TEXTURE2D_DESC desc;
	m_pSwapBackTexture2D->GetDesc(&desc);

	if (desc.Width == rcClient.Width() && desc.Height == rcClient.Height())
		return true;

	ID3D11RenderTargetView *renderView = NULL;
	pDeviceContext->OMSetRenderTargets(1, &renderView, NULL);

	m_pRenderTargetView.Clear();
	m_pSwapBackTexture2D.Clear();
	HRESULT hr = m_pSwapChain->ResizeBuffers(m_SwapBufferCount, rcClient.Width(), rcClient.Height(), SWAP_CHAIN_FORMAT, 0);

	if (FAILED(hr))
		return false;

	return _CreateTargetView(pDevice);
}
