
// RecordMobileye.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CRecordMobileyeApp: 
// �йش����ʵ�֣������ RecordMobileye.cpp
//

class CRecordMobileyeApp : public CWinApp
{
public:
	CRecordMobileyeApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CRecordMobileyeApp theApp;