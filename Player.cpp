#include "stdafx.h"
#include "Player.h"

CPlayer::CPlayer() 
    : m_bExit(false)
    , m_hPreview()
    , m_pSelectedSource()
    , m_bSave(false)
{ 
    SHGetSpecialFolderPathA(0, m_SaveDir, CSIDL_DESKTOP, 0);
}

CPlayer::~CPlayer()
{
}

void CPlayer::_InsertTexture(WCHAR* file)
{ 
	TEXTURE_PTR texture(new tTexture());
    if (texture->InitTexture(m_Render.m_pDevice, file))
    {
        CAutoLockCS alock(m_Lock);
        m_Textures.push_back(texture);
    }
}

void CPlayer::InitPlayer(HWND hPreview)
{
    bool bRes = m_Render.InitRender(hPreview);
    if (!bRes)
    {
        assert(false);
        return;
    }

	_InsertTexture(L"test.png");
    _InsertTexture(L"test1.png");
    _InsertTexture(L"test2.jpg");

    {
        TEXTURE_PTR texture0(new tTexture());
        if (texture0->InitTextureYUV420(m_Render.m_pDevice, m_Render.m_pDeviceContext, "yuv420_720p.yuv"))
        {
            CAutoLockCS alock(m_Lock);
            m_Textures.push_back(texture0);
        }
    }

    {
        TEXTURE_PTR texture0(new tTexture());
        if (texture0->InitTextureYUV420_Ex(m_Render.m_pDevice, m_Render.m_pDeviceContext, "yuv420_720p.yuv"))
        {
            CAutoLockCS alock(m_Lock);
            m_Textures.push_back(texture0);
        }
    }

    {
        TEXTURE_PTR texture1(new tTexture());
        if (texture1->InitTextureYUYV422(m_Render.m_pDevice, m_Render.m_pDeviceContext, "yuyv422.yuv"))
        {
            CAutoLockCS alock(m_Lock);
            m_Textures.push_back(texture1);
        }
    }

    {
        TEXTURE_PTR texture2(new tTexture());
        if (texture2->InitTextureYUYV422(m_Render.m_pDevice, m_Render.m_pDeviceContext, "yuyv422_2.yuv"))
        {
            CAutoLockCS alock(m_Lock);
            m_Textures.push_back(texture2);
        }
    }
     
    m_hPreview = hPreview;

    HANDLE hThreadCheckDevice = (HANDLE)_beginthreadex(0, 0, ThreadFunc, this, 0, 0);
    ::CloseHandle(hThreadCheckDevice);
}

void CPlayer::UninitPlayer()
{
    m_bExit = true;
    Sleep(1000);

    m_Render.UninitRender();

    CAutoLockCS alock(m_Lock);
    m_Textures.clear();
}

void CPlayer::SaveBg()
{
    m_bSave = true;
}

unsigned __stdcall CPlayer::ThreadFunc(void* pParam)
{
    CPlayer* self = reinterpret_cast<CPlayer*>(pParam);
    self->ThreadPlayer(); 
    return 0;
}

void CPlayer::ThreadPlayer()
{  
    while (false == m_bExit)
    {
        m_Render.BeginRender(); 

        {
            CAutoLockCS alock(m_Lock); 
            for (size_t i = 0; i < m_Textures.size(); i++)
            {
                TEXTURE_PTR texture = m_Textures[i];
				m_Render.PrepareRenderTexture(texture.get());
                m_Render.UpdateMatrix(texture->m_WorldMatrix, D3D_BASE_WIDTH, D3D_BASE_HEIGHT);
                m_Render.RenderTexture(texture.get());
                if (texture->m_bSelected)
                    m_Render.RenderBorder(texture.get());
            }
        }

        m_Render.EndRender();

        if (m_bSave)
        {
            m_bSave = false;
            m_Render.ConvertToYUV420(m_SaveDir);
        }

        Sleep(20);
    }
}

CPointF CPlayer::TransToD3D(POINT ptUI)
{
    CRect rcWin;
    ::GetClientRect(m_hPreview, &rcWin);

    float testX = float(rcWin.Width()) / float(D3D_BASE_WIDTH);
    float testY = float(rcWin.Height()) / float(D3D_BASE_HEIGHT); 

    CPointF out;
    out.x = float(ptUI.x) / testX;
    out.y = float(ptUI.y) / testY;
    return out;
}

POINT CPlayer::TransToUI(CPointF ptD3D)
{
    CRect rcWin;
    ::GetClientRect(m_hPreview, &rcWin);

    float testX = float(rcWin.Width()) / float(D3D_BASE_WIDTH);
    float testY = float(rcWin.Height()) / float(D3D_BASE_HEIGHT);

    POINT out;
    out.x = testX * ptD3D.x;
    out.y = testY * ptD3D.y;
    return out;
}

void CPlayer::RemoveSelected()
{
    CAutoLockCS alock(m_Lock);
     
    std::vector<TEXTURE_PTR>::iterator itr = m_Textures.begin();
    for (; itr != m_Textures.end(); ++itr)
    {
        TEXTURE_PTR texture = *itr;
        if (texture->m_bSelected)
        {
            m_Textures.erase(itr); 
            break;
        }
    }
}

TEXTURE_PTR CPlayer::SelectSource(POINT ptUI, E_D3D_MOUSE_STATUS& outMouseState)
{
    CPointF ptD3D = TransToD3D(ptUI);

    TEXTURE_PTR pPreSelected = m_pSelectedSource;
    m_pSelectedSource = TEXTURE_PTR();
    outMouseState = DMS_NONE;
      
    { 
        CAutoLockCS alock(m_Lock);

        int size = (int)m_Textures.size();
        for (int i = size - 1; i >= 0 ; --i)
        {
            TEXTURE_PTR texture = m_Textures[i];
            E_D3D_MOUSE_STATUS eMouseState;
            if (texture->TestSelected(ptD3D, eMouseState))
            { 
                outMouseState = eMouseState; 
                m_pSelectedSource = texture;
                m_ptCursorPosBegin = ptD3D;
                m_ptSourcePosBegin = texture->m_tPosition;
                m_fScaleXBegin = texture->m_tScale.x;
                m_fRightBegin = texture->m_tPosition.x + texture->GetWidthUI();
                break;
            }
        }

        std::vector<TEXTURE_PTR>::iterator itr = m_Textures.begin();
        for (; itr != m_Textures.end(); ++itr)
        {
            TEXTURE_PTR texture = *itr;
            if (texture->m_bSelected)
            {
                m_Textures.erase(itr);
                m_Textures.push_back(texture);
                break;
            }
        } 
    }

    if (pPreSelected.get() && pPreSelected.get() != m_pSelectedSource.get())
        pPreSelected->Select(false);

    return m_pSelectedSource;
}

void CPlayer::OnUIMouseMove(CPoint ptUI, E_D3D_MOUSE_STATUS eMouseState)
{
    if (!m_pSelectedSource.get())
        return;

    CPointF ptD3D = TransToD3D(ptUI);
    if (DMS_MOVE == eMouseState)
    {
        CPointF pos;
        pos.x = m_ptSourcePosBegin.x + (ptD3D.x - m_ptCursorPosBegin.x);
        pos.y = m_ptSourcePosBegin.y + (ptD3D.y - m_ptCursorPosBegin.y);
        m_pSelectedSource->SetPos(pos);
    }
    else if (DMS_RIGHT_BOTTOM == eMouseState)
    {  
        float fCurrent = float(ptD3D.x - m_ptSourcePosBegin.x) + (m_fRightBegin - m_ptCursorPosBegin.x);
        float fPercent = fCurrent / float(m_fRightBegin - m_ptSourcePosBegin.x);
        m_pSelectedSource->SetScale(fPercent * m_fScaleXBegin);
    }
}

E_D3D_MOUSE_STATUS CPlayer::TestMouseStatus(CPoint ptUI)
{
    if (!m_pSelectedSource.get())
        return DMS_NONE;

    CPointF ptD3D = TransToD3D(ptUI);
    return m_pSelectedSource->TestMouseStatus(ptD3D);
}