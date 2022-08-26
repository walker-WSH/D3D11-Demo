
// D3D11-DemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "D3D11-Demo.h"
#include "D3D11-DemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DRAGE_REGION_SIZE 40
#define RESIZE_REGION_SIZE 5

class CAboutDlg : public CDialogEx {
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}

void CAboutDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CD3D11DemoDlg dialog

CD3D11DemoDlg::CD3D11DemoDlg(CWnd *pParent /*=NULL*/) : CDialogEx(IDD_D3D11DEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bLButtonDown = false;
}

void CD3D11DemoDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CD3D11DemoDlg, CDialogEx)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_WM_DESTROY()
ON_WM_NCHITTEST()
ON_WM_NCLBUTTONDOWN()
ON_WM_SETCURSOR()
ON_WM_ERASEBKGND()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONDBLCLK()
ON_WM_RBUTTONDBLCLK()
ON_WM_GETMINMAXINFO()
ON_WM_WINDOWPOSCHANGING()
ON_WM_MBUTTONDBLCLK()
ON_WM_MBUTTONDOWN()
END_MESSAGE_MAP()

BOOL CD3D11DemoDlg::PreTranslateMessage(MSG *pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
		m_Player.SaveBg();
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CD3D11DemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu *pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL) {
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);  // Set big icon
	SetIcon(m_hIcon, FALSE); // Set small icon

	// TODO: Add extra initialization here
	MoveWindow(0, 0, D3D_BASE_WIDTH, D3D_BASE_HEIGHT);
	CenterWindow();

	m_Player.InitPlayer(m_hWnd);

	return TRUE; // return TRUE  unless you set the focus to a control
}

void CD3D11DemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CD3D11DemoDlg::OnPaint()
{
	if (IsIconic()) {
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	} else {
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CD3D11DemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CD3D11DemoDlg::OnDestroy()
{
	// TODO: Add your message handler code here
	ShowWindow(SW_HIDE);
	m_Player.UninitPlayer();

	CDialogEx::OnDestroy();
}

LRESULT CD3D11DemoDlg::OnNcHitTest(CPoint pt)
{
	RECT rcWindow;
	::GetWindowRect(m_hWnd, &rcWindow);

	// 最好将四个角的判断放在前面
	if (pt.x <= rcWindow.left + RESIZE_REGION_SIZE && pt.y <= rcWindow.top + RESIZE_REGION_SIZE)
		return HTTOPLEFT;
	else if (pt.x >= rcWindow.right - RESIZE_REGION_SIZE && pt.y <= rcWindow.top + RESIZE_REGION_SIZE)
		return HTTOPRIGHT;
	else if (pt.x <= rcWindow.left + RESIZE_REGION_SIZE && pt.y >= rcWindow.bottom - RESIZE_REGION_SIZE)
		return HTBOTTOMLEFT;
	else if (pt.x >= rcWindow.right - RESIZE_REGION_SIZE && pt.y >= rcWindow.bottom - RESIZE_REGION_SIZE)
		return HTBOTTOMRIGHT;
	else if (pt.x <= rcWindow.left + RESIZE_REGION_SIZE)
		return HTLEFT;
	else if (pt.x >= rcWindow.right - RESIZE_REGION_SIZE)
		return HTRIGHT;
	else if (pt.y <= rcWindow.top + RESIZE_REGION_SIZE)
		return HTTOP;
	else if (pt.y >= rcWindow.bottom - RESIZE_REGION_SIZE)
		return HTBOTTOM;

	if (pt.y <= (rcWindow.top + DRAGE_REGION_SIZE))
		return HTCAPTION;
	else
		return __super::OnNcHitTest(pt);
}

void CD3D11DemoDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	switch (nHitTest) {
	case HTTOP:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_TOP, MAKELPARAM(point.x, point.y));
		break;
	case HTBOTTOM:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOM, MAKELPARAM(point.x, point.y));
		break;
	case HTLEFT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_LEFT, MAKELPARAM(point.x, point.y));
		break;
	case HTRIGHT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_RIGHT, MAKELPARAM(point.x, point.y));
		break;
	case HTTOPLEFT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPLEFT, MAKELPARAM(point.x, point.y));
		break;
	case HTTOPRIGHT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPRIGHT, MAKELPARAM(point.x, point.y));
		break;
	case HTBOTTOMLEFT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMLEFT, MAKELPARAM(point.x, point.y));
		break;
	case HTBOTTOMRIGHT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMRIGHT, MAKELPARAM(point.x, point.y));
		break;
	default:
		__super::OnNcLButtonDown(nHitTest, point);
	}
}

BOOL CD3D11DemoDlg::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
	if (!m_bLButtonDown) {
		CPoint ptUI;
		GetCursorPos(&ptUI);
		ScreenToClient(&ptUI);

		E_D3D_MOUSE_STATUS status = m_Player.TestMouseStatus(ptUI);
		switch (status) {
		case DMS_MOVE:
			SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL)));
			return TRUE;

		case DMS_RIGHT_BOTTOM:
			SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENWSE)));
			return TRUE;

		default:
			break;
		}
	}

	switch (nHitTest) {
	case HTTOP:
	case HTBOTTOM:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS)));
		return TRUE;

	case HTLEFT:
	case HTRIGHT:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
		return TRUE;

	case HTTOPLEFT:
	case HTBOTTOMRIGHT:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENWSE)));
		return TRUE;

	case HTTOPRIGHT:
	case HTBOTTOMLEFT:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENESW)));
		return TRUE;

	default:
		return __super::OnSetCursor(pWnd, nHitTest, message);
	}
}

BOOL CD3D11DemoDlg::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}

void CD3D11DemoDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_Player.SelectSource(point, m_eMouseState).get()) {
		SetCapture();
		m_bLButtonDown = true;
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CD3D11DemoDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_bLButtonDown)
		m_Player.OnUIMouseMove(point, m_eMouseState);

	CDialogEx::OnMouseMove(nFlags, point);
}

void CD3D11DemoDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_bLButtonDown) {
		m_Player.OnUIMouseMove(point, m_eMouseState);
		ReleaseCapture();
		m_bLButtonDown = false;
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}

void CD3D11DemoDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	E_D3D_MOUSE_STATUS eMouseState;
	TEXTURE_PTR pTexture = m_Player.SelectSource(point, eMouseState);
	if (pTexture.get())
		pTexture->FlipH();

	CDialogEx::OnLButtonDblClk(nFlags, point);
}

void CD3D11DemoDlg::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	E_D3D_MOUSE_STATUS eMouseState;
	TEXTURE_PTR pTexture = m_Player.SelectSource(point, eMouseState);
	if (pTexture.get())
		pTexture->FlipV();

	CDialogEx::OnRButtonDblClk(nFlags, point);
}

void CD3D11DemoDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	E_D3D_MOUSE_STATUS eMouseState;
	TEXTURE_PTR pTexture = m_Player.SelectSource(point, eMouseState);
	if (pTexture.get())
		m_Player.RemoveSelected();

	CDialogEx::OnMButtonDown(nFlags, point);
}

void CD3D11DemoDlg::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnMButtonDblClk(nFlags, point);
}

void CD3D11DemoDlg::OnGetMinMaxInfo(MINMAXINFO *lpMMI)
{
	// TODO: Add your message handler code here and/or call default
	lpMMI->ptMinTrackSize.x = 320;
	lpMMI->ptMinTrackSize.y = 180;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CD3D11DemoDlg::OnWindowPosChanging(WINDOWPOS *lpwndpos)
{
	CDialogEx::OnWindowPosChanging(lpwndpos);

	// TODO: Add your message handler code here
	float ratio = 9.f / 16.f;
	lpwndpos->cx = (lpwndpos->cx / 4 * 4);
	lpwndpos->cy = ratio * float(lpwndpos->cx);
}
