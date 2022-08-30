#pragma once
#include <vector>
#include <Windows.h>
#include "d3dDefine.h"
#include "ImplShader.h"
#include "ImplSwap.h"
#include "ImplTexture.h"

//----------------------------
class CRenderCenter {
public:
	CString m_strAdapterName;

	ComPtr<IDXGIFactory1> m_pFactory1;
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	ComPtr<ID3D11BlendState> m_pBlendState;
	ComPtr<ID3D11SamplerState> m_pSampleState;

	int m_nCurrentSwap;
	std::vector<tWindowSwap> m_WndList;

	tShader *m_pTextureShader;
	tShader m_ShaderRGBA;
	tShader m_ShaderYUYV422;
	tShader m_ShaderYUV420;
	tShader m_ShaderYUV420_Ex;
	tShader m_ShaderBorder;
	tShader m_ShaderRGBA2YUV420;

	D3DXMATRIX m_wvpMatrix;

public:
	CRenderCenter();
	~CRenderCenter();

	bool InitRender(HWND hPreview);
	void UninitRender();

	void BeginRender();
	void PrepareRenderTexture(tTexture *pTexture);
	void UpdateMatrix(D3DXMATRIX worldMatrix, int baseWidth, int baseHeight);
	void RenderTexture(tTexture *pTexture);
	void RenderBorder(tTexture *pTexture);
	void EndRender();

	void ConvertToYUV420(const char *dir);

private:
	bool _InitDevice();
	bool _AddSwapChain(HWND hWnd);
	bool _InitBlendState();
	bool _InitSamplerState();
	void _SetDisplayWnd(tWindowSwap &swap);
	void _UpdateTextureVertexBuffer(int w, int h, bool bFlipH, bool bFlipV);
	void _UpdateBorderVertexBuffer(tTexture *pTexture);
	D3DXMATRIX _GetOrthoMatrix(int baseWidth, int baseHeight);
	void _RenderTextureInner(ID3D11ShaderResourceView **views, int count);

	void _SaveImage(ComPtr<ID3D11Texture2D> pSrc, const char *path);
};
