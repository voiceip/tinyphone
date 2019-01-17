#pragma once

#include "resource.h"

// aboutDialog dialog

class aboutDialog : public CDialogEx
{
	DECLARE_DYNAMIC(aboutDialog)

public:
	aboutDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~aboutDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
