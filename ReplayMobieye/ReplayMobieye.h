
// ReplayMobieye.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CReplayMobieyeApp: 
// �йش����ʵ�֣������ ReplayMobieye.cpp
//

class CReplayMobieyeApp : public CWinApp
{
public:
	CReplayMobieyeApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CReplayMobieyeApp theApp;