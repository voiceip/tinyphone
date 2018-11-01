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

#include "StdAfx.h"
#include "Transfer.h"
#include "mainDlg.h"
#include "langpack.h"

Transfer::Transfer(CWnd* pParent /*=NULL*/)
: CDialog(Transfer::IDD, pParent)
{
	Create (IDD, pParent);
}

Transfer::~Transfer(void)
{
}

int Transfer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (langPack.rtl) {
		ModifyStyleEx(0,WS_EX_LAYOUTRTL);
	}
	return 0;
}

BOOL Transfer::OnInitDialog()
{
	CDialog::OnInitDialog();
	TranslateDialog(this->m_hWnd);
	CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
	CListCtrl *list= (CListCtrl*)mainDlg->pageContacts->GetDlgItem(IDC_CONTACTS);
	if (mainDlg->pageContacts->isFiltered()) {
		mainDlg->pageContacts->filterReset();
	}
	int count = list->GetItemCount();
	for (int i=0;i<count;i++) {
		Contact *pContact = (Contact *) list->GetItemData(i);
		int n = combobox->AddString(pContact->name);
		CString *number = new CString(pContact->number);
		combobox->SetItemData(n, (DWORD_PTR)number);
	}
	combobox->SetWindowText(lastTransferNumber);
	return TRUE;
}

void Transfer::OnDestroy()
{
	mainDlg->transferDlg = NULL;
	CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
	int n = combobox->GetCount();
	for (int i=0;i<n;i++) {
		CString *number = (CString *)combobox->GetItemData(i);
		delete number;
	}
	CDialog::OnDestroy();
}

void Transfer::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}

BEGIN_MESSAGE_MAP(Transfer, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, &Transfer::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &Transfer::OnBnClickedOk)
END_MESSAGE_MAP()


void Transfer::OnClose() 
{
	if (mainDlg->transferDlg) {
		DestroyWindow();
	}
}

void Transfer::OnBnClickedCancel()
{
	OnClose();
}

void Transfer::SetAction(msip_action action)
{
	this->action = action;
	if (action == MSIP_ACTION_TRANSFER) {
		SetWindowText(Translate(_T("Call Transfer")));
	}
	if (action == MSIP_ACTION_INVITE) {
		SetWindowText(Translate(_T("Invite to Conference")));
	}
}

void Transfer::OnBnClickedOk()
{
	CString number;
	CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
	int i = combobox->GetCurSel();
	if (i == -1) {
		combobox->GetWindowText(number);
		number.Trim();
	} else {
		number = * (CString *)combobox->GetItemData(i);
	}
	if (!number.IsEmpty()) {
		lastTransferNumber = number;
		mainDlg->messagesDlg->CallAction(action, number);
		OnClose();
	}
}
