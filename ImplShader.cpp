#include "stdafx.h"
#include "ImplShader.h"
#include <assert.h>

bool tShader::InitShader(ComPtr<ID3D11Device> pDevice, WCHAR *vsFile, WCHAR *psFile, int vertexSize, int vsBufferSize, int psBufferSize)
{
	ComPtr<ID3D10Blob> errorMessage;
	ComPtr<ID3D10Blob> vertexShaderBuffer;
	ComPtr<ID3D10Blob> pixelShaderBuffer;

	UINT flag = D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;

	HRESULT hr = D3DX11CompileFromFile(vsFile, NULL, NULL, "VS", "vs_5_0", flag, 0, NULL, vertexShaderBuffer.Assign(), errorMessage.Assign(), NULL);
	if (FAILED(hr)) {
		if (errorMessage) {
			char *pCompileErrors = (char *)(errorMessage->GetBufferPointer());
			if (pCompileErrors) {
				OutputDebugStringA(pCompileErrors);
				OutputDebugStringA("\n");
			}
		}

		assert(false);
		return false;
	}

	hr = D3DX11CompileFromFile(psFile, NULL, NULL, "PS", "ps_5_0", flag, 0, NULL, pixelShaderBuffer.Assign(), errorMessage.Assign(), NULL);
	if (FAILED(hr)) {
		if (errorMessage) {
			char *pCompileErrors = (char *)(errorMessage->GetBufferPointer());
			if (pCompileErrors) {
				OutputDebugStringA(pCompileErrors);
				OutputDebugStringA("\n");
			}
		}

		assert(false);
		return false;
	}

	hr = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, m_pVertexShader.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	hr = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, m_pPixelShader.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[2] = {};
	inputLayoutDesc[0].SemanticName = "SV_POSITION";
	inputLayoutDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputLayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	inputLayoutDesc[1].SemanticName = "TEXCOORD";
	inputLayoutDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayoutDesc[1].AlignedByteOffset = 16;

	unsigned int numElements = sizeof(inputLayoutDesc) / sizeof(inputLayoutDesc[0]);
	hr = pDevice->CreateInputLayout(inputLayoutDesc, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), m_pInputLayout.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = vertexSize;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	hr = pDevice->CreateBuffer(&vertexBufferDesc, NULL, m_pVertexBuffer.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	if (vsBufferSize > 0) {
		D3D11_BUFFER_DESC CBufferDesc = {};
		CBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		CBufferDesc.ByteWidth = vsBufferSize;
		CBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = pDevice->CreateBuffer(&CBufferDesc, NULL, m_pVSBuffer.Assign());
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
	}

	if (psBufferSize > 0) {
		D3D11_BUFFER_DESC CBufferDesc = {};
		CBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		CBufferDesc.ByteWidth = psBufferSize;
		CBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = pDevice->CreateBuffer(&CBufferDesc, NULL, m_pPSBuffer.Assign());
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
	}

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pDevice->CreateSamplerState(&samplerDesc, m_pSampleState.Assign());
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	return true;
}
