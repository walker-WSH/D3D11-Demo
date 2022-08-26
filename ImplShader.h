#pragma once
#include <vector>
#include <Windows.h>
#include "d3dDefine.h"

struct tShader {
	//vertex shader
	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11Buffer> m_pVSBuffer;

	ComPtr<ID3D11InputLayout> m_pInputLayout;
	ComPtr<ID3D11Buffer> m_pVertexBuffer;

	//pixel shader
	ComPtr<ID3D11PixelShader> m_pPixelShader;
	ComPtr<ID3D11Buffer> m_pPSBuffer;

	ComPtr<ID3D11SamplerState> m_pSampleState;

public:
	bool InitShader(ComPtr<ID3D11Device> pDevice, WCHAR *vsFile, WCHAR *psFile, int vertexSize, int vsBufferSize, int psBufferSize);
};
