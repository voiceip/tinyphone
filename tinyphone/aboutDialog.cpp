// aboutDialog.cpp : implementation file
//

#include "stdafx.h"
#include "aboutDialog.h"
#include "afxdialogex.h"
#include "resource.h"

// aboutDialog dialog

IMPLEMENT_DYNAMIC(aboutDialog, CDialogEx)

aboutDialog::aboutDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ABOUT_DIALOG, pParent)
{

}

aboutDialog::~aboutDialog()
{
}

void aboutDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(aboutDialog, CDialogEx)
END_MESSAGE_MAP()


// aboutDialog message handlers
