
// D3D11-Demo.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CD3D11DemoApp:
// See D3D11-Demo.cpp for the implementation of this class
//

class CD3D11DemoApp : public CWinApp
{
public:
	CD3D11DemoApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CD3D11DemoApp theApp;