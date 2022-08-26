#include "stdafx.h"
#include "ImplTexture.h" 

tTexture::tTexture()
	: m_nWidth(0)
	, m_nHeight(0)
	, m_ShaderType(EST_DEFAULT)
{
	for (size_t i = 0; i < MAX_TEXTURE_NUM; i++)
	{
		m_pTextures[i] = ComPtr<ID3D11Texture2D>();
		m_pResourceViews[i] = ComPtr<ID3D11ShaderResourceView>();
	}

	m_bSelected = false;

	m_bFlipH = m_bFlipV = false;
	m_tPosition.x = m_tPosition.y = 0;
	m_tScale.x = m_tScale.y = 1.f;
	m_fRotate = 0.f;

	_UpdateMatrix();
}

void tTexture::GetResourceViewList(ID3D11ShaderResourceView* out[MAX_TEXTURE_NUM], int& viewCount)
{
	viewCount = 0;
	for (size_t i = 0; i < MAX_TEXTURE_NUM; i++)
	{
		out[i] = m_pResourceViews[i].Get();
		if (out[i])
			++viewCount;
	}
}

void tTexture::_UpdateMatrix()
{
	D3DXMatrixIdentity(&m_WorldMatrix);

	D3DXMATRIX rotate;
	D3DXMatrixIdentity(&rotate);
	rotate.m[0][0] = 0;
	rotate.m[1][1] = 0;
	rotate.m[0][1] = 1.f;
	rotate.m[1][0] = 1.f;
	//m_WorldMatrix *= rotate;

	D3DXMATRIX scale;
	D3DXMatrixIdentity(&scale);
	scale.m[0][0] = m_tScale.x;
	scale.m[1][1] = m_tScale.y;
	m_WorldMatrix *= scale; // 基于旋转后的宽高

	D3DXMATRIX mv;
	D3DXMatrixIdentity(&mv);
	mv.m[3][0] = m_tPosition.x; // 基于D3D坐标系的单位， 不会和scale相乘
	mv.m[3][1] = m_tPosition.y;
	m_WorldMatrix *= mv;
}

bool tTexture::InitTexture(ComPtr<ID3D11Device> pDevice, WCHAR* textureFilename)
{
	HRESULT result = D3DX11CreateShaderResourceViewFromFile(pDevice, textureFilename, NULL, NULL, m_pResourceViews[0].Assign(), NULL);
	if (FAILED(result))
		return false;

	ComPtr<ID3D11Resource> res;
	m_pResourceViews[0]->GetResource(res.Assign());
	if (!res.Get())
		return false;

	res->QueryInterface<ID3D11Texture2D>(m_pTextures[0].Assign());
	if (!m_pTextures[0].Get())
		return false;

	D3D11_TEXTURE2D_DESC desc;
	m_pTextures[0]->GetDesc(&desc);

	m_nWidth = desc.Width;
	m_nHeight = desc.Height;

	_InitPosition();
	return true;
}

bool tTexture::_CreateDynTexture(ComPtr<ID3D11Device> pDevice, int index, DXGI_FORMAT fmt, int w, int h)
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = w;
	desc.Height = h;
	desc.Format = fmt;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.SampleDesc.Count = 1;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;

	HRESULT hr = pDevice->CreateTexture2D(&desc, NULL, m_pTextures[index].Assign());
	if (FAILED(hr))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = {};
	resourceDesc.Format = fmt;
	resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pTextures[index], &resourceDesc, m_pResourceViews[index].Assign());
	if (FAILED(hr))
		return false;

	return true;
}

bool tTexture::InitTextureYUYV422(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pDeviceContext, char* file)
{
	m_ShaderType = EST_YUYV422;

	FILE* fp = 0;
	fopen_s(&fp, file, "rb");
	if (!fp)
		return false;

	fread(&m_nWidth, sizeof(int), 1, fp);
	fread(&m_nHeight, sizeof(int), 1, fp);

	int len = m_nWidth * m_nHeight * 2;
	BYTE* data = new BYTE[len];

	fread(data, len, 1, fp);
	fclose(fp);

	if (_CreateDynTexture(pDevice, 0, DXGI_FORMAT_R8G8B8A8_UNORM, m_nWidth, m_nHeight))
	{
		D3D11_MAPPED_SUBRESOURCE map = {};
		pDeviceContext->Map(m_pTextures[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &map);

		int srcStride = m_nWidth * 2;
		int srcPixSize = 2;

		int destStride = map.RowPitch;
		int destPixSize = 4;

		BYTE preU = 0;
		BYTE preV = 0;
		BYTE* pMapBuffer = reinterpret_cast<BYTE*>(map.pData);
		for (int h = 0; h < m_nHeight; ++h)
		{
			BYTE* srcTemp = data + srcStride * h;
			BYTE* destTemp = pMapBuffer + destStride * h;
			for (int w = 0; w < m_nWidth; ++w)
			{
				destTemp[0] = srcTemp[0]; // y
				if ((w % 2) == 0) // [1] is u
				{ 
					preU = srcTemp[1];
					destTemp[1] = srcTemp[1];
					destTemp[2] = preV;
				}
				else  // [1] is v
				{
					preV = srcTemp[1];
					destTemp[1] = preU;
					destTemp[2] = srcTemp[1];
				}

				srcTemp += srcPixSize;
				destTemp += destPixSize;
			}
		}

		pDeviceContext->Unmap(m_pTextures[0], 0);
	}

	delete[] data;

	_InitPosition();
	return true;
}

bool tTexture::InitTextureYUV420(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pDeviceContext, char* file)
{
	m_ShaderType = EST_YUV420;

	FILE* fp = 0;
	fopen_s(&fp, file, "rb");
	if (!fp)
		return false;

	fread(&m_nWidth, sizeof(int), 1, fp);
	fread(&m_nHeight, sizeof(int), 1, fp);

	if (_CreateDynTexture(pDevice, 0, DXGI_FORMAT_R8G8B8A8_UNORM, m_nWidth, m_nHeight))
	{
		int yLen = m_nWidth * m_nHeight;
		int uLen = (m_nWidth * m_nHeight) / 4;
		int vLen = (m_nWidth * m_nHeight) / 4;

		BYTE* yData = new BYTE[yLen];
		BYTE* uData = new BYTE[uLen];
		BYTE* vData = new BYTE[vLen];

		fread(yData, yLen, 1, fp);
		fread(uData, uLen, 1, fp);
		fread(vData, vLen, 1, fp);

		D3D11_MAPPED_SUBRESOURCE map = {};
		pDeviceContext->Map(m_pTextures[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &map);

		BYTE* pMapBuffer = reinterpret_cast<BYTE*>(map.pData);
		int strideY = m_nWidth;
		int strideUV = m_nWidth / 2;
		int pixelSize = 4;
		for (int h = 0; h < m_nHeight; h += 2)
		{
			for (int w = 0; w < m_nWidth; w += 2)
			{
				int h2 = h / 2;
				int w2 = w / 2;
				int indexUV = strideUV * h2 + w2;

				BYTE u = uData[indexUV];
				BYTE v = vData[indexUV];

				for (int h2 = 0; h2 < 2 && (h + h2) < m_nHeight; h2++)
				{
					int dest = map.RowPitch * (h + h2);
					for (int w2 = 0; w2 < 2 && (w + w2) < m_nWidth; w2++)
					{
						BYTE y = yData[strideY * (h + h2) + (w + w2)];
						pMapBuffer[dest + (w + w2) * pixelSize + 0] = y; // r y
						pMapBuffer[dest + (w + w2) * pixelSize + 1] = u; // g u
						pMapBuffer[dest + (w + w2) * pixelSize + 2] = v; // b v
					}
				}
			}
		}

		pDeviceContext->Unmap(m_pTextures[0], 0);
		delete[] yData;
		delete[] uData;
		delete[] vData;
	}

	fclose(fp);

	_InitPosition();
	return true;
}

void tTexture::_MapTexture(ComPtr<ID3D11DeviceContext> pDeviceContext, int index, BYTE* src, int width, int height)
{
	D3D11_MAPPED_SUBRESOURCE map = {};
	pDeviceContext->Map(m_pTextures[index], 0, D3D11_MAP_WRITE_DISCARD, 0, &map);

    int srcStride = width;
    int destStride = map.RowPitch;
	BYTE* pMapBuffer = reinterpret_cast<BYTE*>(map.pData);
	for (int h = 0; h < height; h += 1)
	{
        memmove(pMapBuffer, src, srcStride);
        pMapBuffer += destStride;
        src += srcStride;
	}

	pDeviceContext->Unmap(m_pTextures[index], 0);
}

bool tTexture::InitTextureYUV420_Ex(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pDeviceContext, char* file)
{
	m_ShaderType = EST_YUV420_Ex;

	FILE* fp = 0;
	fopen_s(&fp, file, "rb");
	if (!fp)
		return false;

	fread(&m_nWidth, sizeof(int), 1, fp);
	fread(&m_nHeight, sizeof(int), 1, fp);

	if (!_CreateDynTexture(pDevice, 0, DXGI_FORMAT_R8_UNORM, m_nWidth, m_nHeight) ||
		!_CreateDynTexture(pDevice, 1, DXGI_FORMAT_R8_UNORM, m_nWidth / 2, m_nHeight / 2) ||
		!_CreateDynTexture(pDevice, 2, DXGI_FORMAT_R8_UNORM, m_nWidth / 2, m_nHeight / 2))
	{
		fclose(fp);
		return false;
	}

	int yLen = m_nWidth * m_nHeight;
	int uLen = (m_nWidth * m_nHeight) / 4;
	int vLen = (m_nWidth * m_nHeight) / 4;

	BYTE* yData = new BYTE[yLen];
	BYTE* uData = new BYTE[uLen];
	BYTE* vData = new BYTE[vLen];

	fread(yData, yLen, 1, fp);
	fread(uData, uLen, 1, fp);
	fread(vData, vLen, 1, fp);
	fclose(fp);

	_MapTexture(pDeviceContext, 0, yData, m_nWidth, m_nHeight);
	_MapTexture(pDeviceContext, 1, uData, m_nWidth / 2, m_nHeight / 2);
	_MapTexture(pDeviceContext, 2, vData, m_nWidth / 2, m_nHeight / 2);

	delete[] yData;
	delete[] uData;
	delete[] vData;

	_InitPosition();
	return true;
}

void tTexture::_InitPosition()
{
	float scale;
	float winR = float(D3D_BASE_WIDTH) / float(D3D_BASE_HEIGHT);
	float imgR = float(m_nWidth) / float(m_nHeight);
	if (winR < imgR)
		scale = float(D3D_BASE_WIDTH) / float(m_nWidth);
	else
		scale = float(D3D_BASE_HEIGHT) / float(m_nHeight);

	m_tScale.x = m_tScale.y = scale;

	int imageW = scale * (float)m_nWidth;
	int imageH = scale * (float)m_nHeight;
	m_tPosition.x = float(D3D_BASE_WIDTH - imageW) / 2;
	m_tPosition.y = float(D3D_BASE_HEIGHT - imageH) / 2;

	_UpdateMatrix();
}

#define BORDER_OFFSET  10 
E_D3D_MOUSE_STATUS tTexture::TestMouseStatus(CPointF ptD3D)
{
	float right = m_tPosition.x + GetWidthUI();
	float bottom = m_tPosition.y + GetHeightUI();
	if (ptD3D.x >= m_tPosition.x &&
		ptD3D.x <= right &&
		ptD3D.y >= m_tPosition.y &&
		ptD3D.y <= bottom)
	{
		if (ptD3D.x >= (right - BORDER_OFFSET) && ptD3D.y >= (bottom - BORDER_OFFSET))
			return DMS_RIGHT_BOTTOM;
		else
			return DMS_MOVE;
	}
	else
	{
		return DMS_NONE;
	}
}

bool tTexture::TestSelected(CPointF ptD3D, E_D3D_MOUSE_STATUS& outMouseState)
{
	outMouseState = TestMouseStatus(ptD3D);
	if (outMouseState != DMS_NONE)
	{
		m_bSelected = true;
	}
	else
	{
		m_bSelected = false;
	}

	return m_bSelected;
}

void tTexture::Select(bool bSelected)
{
	m_bSelected = bSelected;
}

float tTexture::GetWidthUI()
{
	return m_tScale.x * m_nWidth;
}

float tTexture::GetHeightUI()
{
	return m_tScale.y * m_nHeight;
}

void tTexture::SetScale(float fScale)
{
	m_tScale.x = m_tScale.y = fScale;
	_UpdateMatrix();
}

void tTexture::SetPos(CPointF pos)
{
	m_tPosition = pos;
	_UpdateMatrix();
}

void tTexture::FlipH()
{
	m_bFlipH = !m_bFlipH;
}

void tTexture::FlipV()
{
	m_bFlipV = !m_bFlipV;
}