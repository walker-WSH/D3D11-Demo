#include "stdafx.h"
#include "RenderCenter.h"

static D3D_DRIVER_TYPE driverTypes[] = {
	D3D_DRIVER_TYPE_HARDWARE,
	D3D_DRIVER_TYPE_WARP,
	D3D_DRIVER_TYPE_REFERENCE,
};

static D3D_FEATURE_LEVEL featureLevels[] = {
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_3,
};

//----------------------------------------------
CRenderCenter::CRenderCenter() : m_pTextureShader(0) {}

CRenderCenter::~CRenderCenter() {}

bool CRenderCenter::InitRender(HWND hPreview)
{
	UninitRender();

	if (!_InitDevice())
		return false;

	if (!_InitBlendState())
		return false;

	if (!_InitSamplerState())
		return false;

	if (!_AddSwapChain(hPreview)) // create default display
		return false;

	if (!m_ShaderRGBA.InitShader(m_pDevice, L"shader\\texture.vs", L"shader\\texture.ps", sizeof(tTextureVertexType) * TEXTURE_VERTEX_COUNT, sizeof(D3DXMATRIX), 0)) {
		return false;
	}

	if (!m_ShaderRGBA2YUV420.InitShader(m_pDevice, L"shader\\texture.vs", L"shader\\rgba2yuv420.ps", sizeof(tTextureVertexType) * TEXTURE_VERTEX_COUNT, sizeof(D3DXMATRIX), sizeof(float) * 4)) {
		return false;
	}

	if (!m_ShaderYUYV422.InitShader(m_pDevice, L"shader\\texture.vs", L"shader\\yuv422.ps", sizeof(tTextureVertexType) * TEXTURE_VERTEX_COUNT, sizeof(D3DXMATRIX), sizeof(float) * 4)) {
		return false;
	}

	if (!m_ShaderYUV420.InitShader(m_pDevice, L"shader\\texture.vs", L"shader\\yuv420.ps", sizeof(tTextureVertexType) * TEXTURE_VERTEX_COUNT, sizeof(D3DXMATRIX), 0)) {
		return false;
	}

	if (!m_ShaderYUV420_Ex.InitShader(m_pDevice, L"shader\\texture.vs", L"shader\\yuv420_Ex.ps", sizeof(tTextureVertexType) * TEXTURE_VERTEX_COUNT, sizeof(D3DXMATRIX), 0)) {
		return false;
	}

	if (!m_ShaderBorder.InitShader(m_pDevice, L"shader\\border.vs", L"shader\\border.ps", sizeof(tBorderVertexType) * BORDER_VERTEX_COUNT, sizeof(D3DXMATRIX), sizeof(float) * 4)) {
		return false;
	}

	return true;
}

void CRenderCenter::UninitRender()
{
	m_WndList.clear();
}

bool CRenderCenter::_InitBlendState()
{
	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;

	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (FAILED(m_pDevice->CreateBlendState(&blendStateDescription, m_pBlendState.Assign()))) {
		assert(false);
		return false;
	}

	return true;
}

bool CRenderCenter::_InitSamplerState()
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	if (FAILED(m_pDevice->CreateSamplerState(&samplerDesc, m_pSampleState.Assign()))) {
		assert(false);
		return false;
	}

	return true;
}

void CRenderCenter::_SetDisplayWnd(tWindowSwap &swap)
{
	ID3D11RenderTargetView *view = swap.m_pRenderTargetView.Get();
	m_pDeviceContext->OMSetRenderTargets(1, &view, NULL);

	CRect rcClient;
	::GetClientRect(swap.m_hWnd, &rcClient);

	D3D11_VIEWPORT vp;
	memset(&vp, 0, sizeof(vp));
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = (float)0;
	vp.TopLeftY = (float)0;
	vp.Width = (float)rcClient.Width();
	vp.Height = (float)rcClient.Height();
	m_pDeviceContext->RSSetViewports(1, &vp);
}

bool CRenderCenter::_InitDevice()
{
	HRESULT hr = S_OK;

	UINT numDriverTypes = ARRAYSIZE(driverTypes);
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);
	D3D_FEATURE_LEVEL levelUsed = D3D_FEATURE_LEVEL_9_3;
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		hr = D3D11CreateDevice(nullptr, driverTypes[driverTypeIndex], nullptr, 0, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, m_pDevice.Assign(), &levelUsed,
				       m_pDeviceContext.Assign());

		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return false;

	ComPtr<IDXGIDevice> dxgiDevice;
	hr = m_pDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(dxgiDevice.Assign()));
	if (SUCCEEDED(hr)) {
		ComPtr<IDXGIAdapter> adapter;
		hr = dxgiDevice->GetAdapter(adapter.Assign());
		if (SUCCEEDED(hr)) {
			DXGI_ADAPTER_DESC adapterDesc = {};
			adapter->GetDesc(&adapterDesc);
			m_strAdapterName = adapterDesc.Description;

			hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(m_pFactory1.Assign()));
		}
	}

	if (FAILED(hr))
		return false;

	return true;
}

bool CRenderCenter::_AddSwapChain(HWND hWnd)
{
	tWindowSwap swap;
	if (!swap.InitSwap(m_pFactory1, m_pDevice, hWnd))
		return false;

	m_WndList.push_back(swap);
	m_nCurrentSwap = m_WndList.size() - 1;

	_SetDisplayWnd(m_WndList[m_nCurrentSwap]);
	return true;
}

D3DXMATRIX CRenderCenter::_GetOrthoMatrix(int baseWidth, int baseHeight)
{
	FLOAT zn = -100.f;
	FLOAT zf = 100.f;
	D3DXMATRIX orthoMatrix;
	if (0) {
		// 方法一：
		memset(orthoMatrix.m, 0, sizeof(orthoMatrix.m));
		orthoMatrix.m[0][0] = 2 / ((float)baseWidth);
		orthoMatrix.m[1][1] = 2 / (-1.0f * baseHeight);
		orthoMatrix.m[2][2] = 1 / (float)(zf - zn);
		orthoMatrix.m[3][0] = -1.0f;
		orthoMatrix.m[3][1] = 1.0f;
		orthoMatrix.m[3][2] = 0.5f;
		orthoMatrix.m[3][3] = 1.0f;
	} else {
		// 方法二：
		D3DXMatrixOrthoLH(&orthoMatrix, (float)baseWidth, (float)baseHeight, zn, zf);
		orthoMatrix.m[1][1] = -orthoMatrix.m[1][1];
		orthoMatrix.m[3][0] = -1.0f;
		orthoMatrix.m[3][1] = 1.0f;
	}

	return orthoMatrix;
}

void CRenderCenter::BeginRender()
{
	tWindowSwap &swap = m_WndList[m_nCurrentSwap];
	if (swap.TestResizeSwapChain(m_pDevice, m_pDeviceContext))
		_SetDisplayWnd(swap);

	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_pDeviceContext->OMSetBlendState(m_pBlendState, blendFactor, 0xffffffff);

	float color[4] = {0, 0, 0, 0};
	m_pDeviceContext->ClearRenderTargetView(m_WndList[m_nCurrentSwap].m_pRenderTargetView, color);
}

void CRenderCenter::EndRender()
{
	m_WndList[m_nCurrentSwap].m_pSwapChain->Present(0, 0);
}

void CRenderCenter::PrepareRenderTexture(tTexture *pTexture)
{
	switch (pTexture->m_ShaderType) {
	case EST_YUYV422: {
		float wh[2] = {(float)pTexture->m_nWidth, (float)pTexture->m_nHeight};
		m_pDeviceContext->UpdateSubresource(m_ShaderYUYV422.m_pPSBuffer, 0, nullptr, wh, 0, 0);
	}
		m_pTextureShader = &m_ShaderYUYV422;
		break;

	case EST_YUV420:
		m_pTextureShader = &m_ShaderYUV420;
		break;

	case EST_YUV420_Ex:
		m_pTextureShader = &m_ShaderYUV420_Ex;
		break;

	case EST_DEFAULT:
	default:
		m_pTextureShader = &m_ShaderRGBA;
		break;
	}
}

void CRenderCenter::UpdateMatrix(D3DXMATRIX worldMatrix, int baseWidth, int baseHeight)
{
	worldMatrix.m[0][2] = -worldMatrix.m[0][2];
	worldMatrix.m[1][2] = -worldMatrix.m[1][2];
	worldMatrix.m[2][2] = -worldMatrix.m[2][2];
	worldMatrix.m[3][2] = -worldMatrix.m[3][2];

	m_wvpMatrix = worldMatrix * _GetOrthoMatrix(baseWidth, baseHeight);
	D3DXMatrixTranspose(&m_wvpMatrix, &m_wvpMatrix);

	m_pDeviceContext->UpdateSubresource(m_pTextureShader->m_pVSBuffer, 0, nullptr, &(m_wvpMatrix.m[0][0]), 0, 0);
}

void SwapFloat(float &f1, float &f2)
{
	float temp = f1;
	f1 = f2;
	f2 = temp;
}

void CRenderCenter::_UpdateTextureVertexBuffer(int w, int h, bool bFlipH, bool bFlipV)
{
	float left = 0;
	float right = left + (float)w;
	float top = 0;
	float bottom = top + (float)h;

	float leftUV = 0.f;
	float rightUV = 1.f;
	float topUV = 0.f;
	float bottomUV = 1.f;

	if (bFlipH)
		SwapFloat(leftUV, rightUV);

	if (bFlipV)
		SwapFloat(topUV, bottomUV);

	tTextureVertexType vertex[TEXTURE_VERTEX_COUNT];
	vertex[0] = {left, top, 0, 1.f, leftUV, topUV};
	vertex[1] = {right, top, 0, 1.f, rightUV, topUV};
	vertex[2] = {left, bottom, 0, 1.f, leftUV, bottomUV};
	vertex[3] = {right, bottom, 0, 1.f, rightUV, bottomUV};

	m_pDeviceContext->UpdateSubresource(m_pTextureShader->m_pVertexBuffer, 0, nullptr, &vertex, 0, 0);
}

void CRenderCenter::RenderTexture(tTexture *pTexture)
{
	int viewCount;
	ID3D11ShaderResourceView *out[MAX_TEXTURE_NUM] = {};
	pTexture->GetResourceViewList(out, viewCount);

	_UpdateTextureVertexBuffer(pTexture->m_nWidth, pTexture->m_nHeight, pTexture->m_bFlipH, pTexture->m_bFlipV);
	_RenderTextureInner(out, viewCount);
}

void CRenderCenter::_RenderTextureInner(ID3D11ShaderResourceView **views, int count)
{
	unsigned int stride = sizeof(tTextureVertexType);
	unsigned int offset = 0;
	ID3D11Buffer *buffer[1];

	buffer[0] = m_pTextureShader->m_pVertexBuffer;
	m_pDeviceContext->IASetVertexBuffers(0, 1, buffer, &stride, &offset);
	m_pDeviceContext->IASetInputLayout(m_pTextureShader->m_pInputLayout);

	m_pDeviceContext->VSSetShader(m_pTextureShader->m_pVertexShader, NULL, 0);
	if (m_pTextureShader->m_pVSBuffer.Get()) {
		buffer[0] = m_pTextureShader->m_pVSBuffer;
		m_pDeviceContext->VSSetConstantBuffers(0, 1, buffer);
	}

	m_pDeviceContext->PSSetShader(m_pTextureShader->m_pPixelShader, NULL, 0);
	if (m_pTextureShader->m_pPSBuffer.Get()) {
		buffer[0] = m_pTextureShader->m_pPSBuffer;
		m_pDeviceContext->PSSetConstantBuffers(0, 1, buffer);
	}

	ID3D11SamplerState *sampleState = m_pSampleState.Get();
	m_pDeviceContext->PSSetSamplers(0, 1, &sampleState);
	m_pDeviceContext->PSSetShaderResources(0, count, views);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pDeviceContext->Draw(TEXTURE_VERTEX_COUNT, 0);
}

void CRenderCenter::_UpdateBorderVertexBuffer(tTexture *pTexture)
{
	float left = 0;
	float right = left + (float)pTexture->m_nWidth;
	float top = 0;
	float bottom = top + (float)pTexture->m_nHeight;

	tBorderVertexType vertex[BORDER_VERTEX_COUNT];
	vertex[0] = {left, top, 0, 1.f};
	vertex[1] = {left, bottom, 0, 1.f};
	vertex[2] = {right, bottom, 0, 1.f};
	vertex[3] = {right, top, 0, 1.f};
	vertex[4] = {left, top, 0, 1.f};

	m_pDeviceContext->UpdateSubresource(m_ShaderBorder.m_pVertexBuffer, 0, nullptr, &vertex, 0, 0);

	m_pDeviceContext->UpdateSubresource(m_ShaderBorder.m_pVSBuffer, 0, nullptr, &(m_wvpMatrix.m[0][0]), 0, 0);

	float color[] = {1.f, 1.f, 0, 1.f};
	m_pDeviceContext->UpdateSubresource(m_ShaderBorder.m_pPSBuffer, 0, nullptr, color, 0, 0);
}

void CRenderCenter::RenderBorder(tTexture *pTexture)
{
	_UpdateBorderVertexBuffer(pTexture);

	unsigned int stride = sizeof(tBorderVertexType);
	unsigned int offset = 0;
	ID3D11Buffer *buffer[1];

	buffer[0] = m_ShaderBorder.m_pVertexBuffer;
	m_pDeviceContext->IASetVertexBuffers(0, 1, buffer, &stride, &offset);
	m_pDeviceContext->IASetInputLayout(m_ShaderBorder.m_pInputLayout);

	m_pDeviceContext->VSSetShader(m_ShaderBorder.m_pVertexShader, NULL, 0);
	if (m_ShaderBorder.m_pVSBuffer.Get()) {
		buffer[0] = m_ShaderBorder.m_pVSBuffer;
		m_pDeviceContext->VSSetConstantBuffers(0, 1, buffer);
	}

	m_pDeviceContext->PSSetShader(m_ShaderBorder.m_pPixelShader, NULL, 0);
	if (m_ShaderBorder.m_pPSBuffer.Get()) {
		buffer[0] = m_ShaderBorder.m_pPSBuffer;
		m_pDeviceContext->PSSetConstantBuffers(0, 1, buffer);
	}

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	m_pDeviceContext->Draw(BORDER_VERTEX_COUNT, 0);
}

void CRenderCenter::_SaveImage(ComPtr<ID3D11Texture2D> pSrc, const char *path)
{
	D3D11_TEXTURE2D_DESC desc = {};
	pSrc->GetDesc(&desc);

	desc.BindFlags = 0;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	ComPtr<ID3D11Texture2D> destTexture;
	HRESULT hr = m_pDevice->CreateTexture2D(&desc, NULL, destTexture.Assign());
	if (FAILED(hr))
		return;

	m_pDeviceContext->CopyResource(destTexture, pSrc);

	D3D11_MAPPED_SUBRESOURCE map = {};
	hr = m_pDeviceContext->Map(destTexture, 0, D3D11_MAP_READ, 0, &map);
	if (FAILED(hr))
		return;

#if 1
	int wxh = desc.Width * desc.Height;

	BYTE *y0;
	BYTE *y1;
	BYTE *u0;
	BYTE *u1;
	BYTE *v0;
	BYTE *v1;
	y0 = y1 = new BYTE[wxh];
	u0 = u1 = new BYTE[wxh / 4];
	v0 = v1 = new BYTE[wxh / 4];

	int cnt = 0;
	BYTE *pData = (BYTE *)map.pData;
	for (int i = 0; i < desc.Height; i += 1) {
		BYTE *temp = pData + i * map.RowPitch;
		for (int j = 0; j < desc.Width; j += 1) {
			*y1 = temp[0];
			y1++;

			if ((i % 2) == 0 && (j % 2) == 0) {
				*u1 = temp[1];
				u1++;

				*v1 = temp[2];
				v1++;

				cnt++;
			}

			temp += 4;
		}
	}

	FILE *fp = 0;
	fopen_s(&fp, path, "wb+");
	if (fp) {
		// fwrite(&desc.Width, 4, 1, fp);
		// fwrite(&desc.Height, 4, 1, fp);
		fwrite(y0, wxh, 1, fp);
		fwrite(u0, wxh / 4, 1, fp);
		fwrite(v0, wxh / 4, 1, fp);
		fclose(fp);
	}

	delete[] y0;
	delete[] u0;
	delete[] v0;
#endif

	m_pDeviceContext->Unmap(destTexture, 0);
}

void CRenderCenter::ConvertToYUV420(const char *dir)
{
	D3D11_TEXTURE2D_DESC bkDesc = {};
	m_WndList[m_nCurrentSwap].m_pSwapBackTexture2D->GetDesc(&bkDesc);
	bkDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bkDesc.Usage = D3D11_USAGE_DYNAMIC;
	bkDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = {};
	resourceDesc.Format = bkDesc.Format;
	resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceDesc.Texture2D.MipLevels = 1;

	ComPtr<ID3D11Texture2D> pSwapBackTexture2D;
	ComPtr<ID3D11ShaderResourceView> pBkResourceViews;

	ComPtr<ID3D11Texture2D> pCanvasTextures;
	ComPtr<ID3D11RenderTargetView> pCanvasView;

	HRESULT hr = m_pDevice->CreateTexture2D(&bkDesc, NULL, pSwapBackTexture2D.Assign());
	if (FAILED(hr))
		return;

	hr = m_pDevice->CreateShaderResourceView(pSwapBackTexture2D, &resourceDesc, pBkResourceViews.Assign());
	if (FAILED(hr))
		return;

	m_pDeviceContext->CopyResource(pSwapBackTexture2D, m_WndList[m_nCurrentSwap].m_pSwapBackTexture2D);

	D3D11_TEXTURE2D_DESC td = {};
	td.Width = bkDesc.Width;
	td.Height = bkDesc.Height;
	td.Format = bkDesc.Format;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	td.SampleDesc.Count = 1;
	td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	td.Usage = D3D11_USAGE_DEFAULT;

	hr = m_pDevice->CreateTexture2D(&td, NULL, pCanvasTextures.Assign());
	if (FAILED(hr))
		return;

	hr = m_pDevice->CreateRenderTargetView(pCanvasTextures, NULL, pCanvasView.Assign());
	if (FAILED(hr))
		return;

	m_pTextureShader = &m_ShaderRGBA2YUV420;

	ID3D11RenderTargetView *view = pCanvasView.Get();
	m_pDeviceContext->OMSetRenderTargets(1, &view, NULL);

	D3D11_VIEWPORT vp;
	memset(&vp, 0, sizeof(vp));
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = (float)0;
	vp.TopLeftY = (float)0;
	vp.Width = (float)td.Width;
	vp.Height = (float)td.Height;
	m_pDeviceContext->RSSetViewports(1, &vp);

	float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	m_pDeviceContext->OMSetBlendState(m_pBlendState, blendFactor, 0xffffffff);

	float color[4] = {0, 0, 0, 255};
	m_pDeviceContext->ClearRenderTargetView(pCanvasView, color);

	float wh[] = {(float)td.Width, (float)td.Height, (1.f / (float)td.Width) / 2.f, (1.f / (float)td.Height) / 2.f};
	m_pDeviceContext->UpdateSubresource(m_pTextureShader->m_pPSBuffer, 0, nullptr, wh, 0, 0);

	D3DXMATRIX worldMatrix;
	D3DXMatrixIdentity(&worldMatrix);
	UpdateMatrix(worldMatrix, td.Width, td.Height);

	ID3D11ShaderResourceView *out[] = {pBkResourceViews.Get()};
	_UpdateTextureVertexBuffer(td.Width, td.Height, false, false);
	_RenderTextureInner(out, 1);

	//-------------------------------------------------------
	char path[MAX_PATH + 1];
	sprintf_s(path, "%s\\%dx%d_420.yuv", dir, bkDesc.Width, bkDesc.Height);

	_SaveImage(pCanvasTextures, path);
	_SetDisplayWnd(m_WndList[m_nCurrentSwap]);
}
