#pragma once
#include <d3d11.h> 
#include <d3dx11async.h> 
#include <d3dx10math.h>
#include "ComPtr.h"

#pragma comment(lib, "d3d11.lib") 
#pragma comment(lib, "D3DX11.lib") 
#pragma comment(lib, "D3DX10.lib")  

// D3D画布基准宽高，窗口像素坐标需要转换到D3D的画布坐标系
#define D3D_BASE_WIDTH   1280
#define D3D_BASE_HEIGHT  720

#define SAFE_RELEASE_COM(x)   if(x) {x->Release(); x = NULL;}

#define TEXTURE_VERTEX_COUNT  4
#define BORDER_VERTEX_COUNT   5

struct tTextureVertexType
{
    float x, y, z, w;
    float u, v;
};

struct tBorderVertexType
{
    float x, y, z, w;
};

struct CPointF
{
    float x;
    float y;
};

enum E_D3D_MOUSE_STATUS
{
    DMS_NONE = 0,
    DMS_MOVE,
    DMS_RIGHT_BOTTOM,
};