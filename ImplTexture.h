#pragma once
#include <vector>
#include <Windows.h>
#include "d3dDefine.h"

#define MAX_TEXTURE_NUM  8

enum E_SHADER_TYPE
{
	EST_DEFAULT = 0,
    EST_YUYV422,
	EST_YUV420,
    EST_YUV420_Ex,
};

struct tTexture
{
	E_SHADER_TYPE m_ShaderType;

	bool m_bSelected;

	bool m_bFlipH;
	bool m_bFlipV;
	CPointF m_tPosition;
	CPointF m_tScale;
	float m_fRotate;
	D3DXMATRIX m_WorldMatrix;

	int m_nWidth;
	int m_nHeight;
	ComPtr<ID3D11Texture2D> m_pTextures[MAX_TEXTURE_NUM];
	ComPtr<ID3D11ShaderResourceView> m_pResourceViews[MAX_TEXTURE_NUM]; 

public:
	tTexture();

	bool InitTexture(ComPtr<ID3D11Device> pDevice, WCHAR* textureFilename);
	bool InitTextureYUYV422(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pDeviceContext, char* file);
	bool InitTextureYUV420(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pDeviceContext, char* file);
    bool InitTextureYUV420_Ex(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pDeviceContext, char* file);

	void GetResourceViewList(ID3D11ShaderResourceView* out[MAX_TEXTURE_NUM], int& viewCount);

	bool TestSelected(CPointF ptD3D, E_D3D_MOUSE_STATUS& outMouseState);
	E_D3D_MOUSE_STATUS TestMouseStatus(CPointF ptD3D);

	void Select(bool bSelected);
	void SetPos(CPointF pos);
	void SetScale(float fScale);
	void FlipH();
	void FlipV();

	float GetWidthUI();
	float GetHeightUI();

private:
	void _InitPosition();
	void _UpdateMatrix();
	bool _CreateDynTexture(ComPtr<ID3D11Device> pDevice, int index, DXGI_FORMAT fmt, int w, int h);
    void _MapTexture(ComPtr<ID3D11DeviceContext> pDeviceContext, int index, BYTE* src, int width, int height);
};