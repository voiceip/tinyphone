/* 
 * Copyright (C) 2011-2018 MicroSIP (http://www.microsip.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include "stdafx.h"
#include "microsip.h"
#include "AddDlg.h"
#include "mainDlg.h"
#include "langpack.h"

AddDlg::AddDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AddDlg::IDD, pParent)
{
	Create (IDD, pParent);
}

AddDlg::~AddDlg()
{
}

int AddDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (langPack.rtl) {
		ModifyStyleEx(0,WS_EX_LAYOUTRTL);
	}
	return 0;
}

BOOL AddDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	TranslateDialog(this->m_hWnd);
	return TRUE;
}

void AddDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}


BEGIN_MESSAGE_MAP(AddDlg, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, &AddDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &AddDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


void AddDlg::OnClose() 
{
	this->ShowWindow(SW_HIDE);
}

void AddDlg::OnBnClickedOk()
{
	CString number;
	CString name;
	BOOL presence;
			
	GetDlgItem(IDC_EDIT_NUMBER)->GetWindowText(number);
	number=number.Trim();
	if (number.GetLength()) {
		GetDlgItem(IDC_EDIT_NAME)->GetWindowText(name);
		name=name.Trim();
		name=name.GetLength()?name:number;
		if (listIndex != -1) {
			mainDlg->pageContacts->ContactDelete(listIndex);
		}
		presence = ((CButton*)GetDlgItem(IDC_PRESENCE))->GetCheck();
		mainDlg->pageContacts->ContactAdd(number, name, presence, -1, TRUE);
		OnClose();
	}
}

void AddDlg::OnBnClickedCancel()
{
	OnClose();
}
