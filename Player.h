#pragma once
#include "RenderCenter.h"
#include <Windows.h>
#include <process.h>
#include <assert.h>
#include <memory>
#include "lock.h"

typedef std::shared_ptr<tTexture> TEXTURE_PTR;

class CPlayer
{
    bool m_bExit;
    HWND m_hPreview;
    CRenderCenter m_Render;

    CCSetion m_Lock;
    TEXTURE_PTR m_pSelectedSource;
    std::vector<TEXTURE_PTR> m_Textures;

    CPointF m_ptCursorPosBegin;
    CPointF m_ptSourcePosBegin;
    CPointF m_ptSourceScaleBegin;
    float m_fScaleXBegin;
    float m_fRightBegin;

    bool m_bSave;
    char m_SaveDir[_MAX_PATH + 1];

public:
    CPlayer();
    ~CPlayer();

    void InitPlayer(HWND hPreview);
    void UninitPlayer();

public:
    CPointF TransToD3D(POINT ptUI);
    POINT TransToUI(CPointF ptD3D);

    void SaveBg();
    void RemoveSelected();
    TEXTURE_PTR SelectSource(POINT ptUI, E_D3D_MOUSE_STATUS& outMouseState);
    void OnUIMouseMove(CPoint ptUI, E_D3D_MOUSE_STATUS eMouseState);
    E_D3D_MOUSE_STATUS TestMouseStatus(CPoint ptUI);

private:
    static unsigned __stdcall ThreadFunc(void* pParam);
    void ThreadPlayer();

	void _InsertTexture(WCHAR* file);
}; 