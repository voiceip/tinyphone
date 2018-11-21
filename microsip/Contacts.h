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

#pragma once

#include "resource.h"
#include "global.h"
#include "AddDlg.h"
#include "BaseDialog.h"
#include "CListCtrl_SortItemsEx.h"

class Contacts :
	public CBaseDialog
{
public:
	Contacts(CWnd* pParent = NULL);	// standard constructor
	~Contacts();
	enum { IDD = IDD_CONTACTS };

	CListCtrl_SortItemsEx m_SortItemsExListCtrl;

	AddDlg* addDlg;
	BOOL isSubscribed;

	bool ContactAdd(CString number, CString name, char presence, char directory, BOOL save = FALSE, BOOL fromDirectory = FALSE);

	void ContactDelete(int i);
	void ContactsSave();
	void ContactsLoad();
	void ContactsClear();
	bool isFiltered(Contact *pContact = NULL);
	void filterReset();

	void SetCanditates();
	int DeleteCanditates();

	void UpdateCallButton();
	CString GetNameByNumber(CString number);
	void PresenceSubsribeOne(Contact *pContact);
	void PresenceUnsubsribeOne(Contact *pContact);
	void PresenceSubsribe();
	void PresenceUnsubsribe();

	void OnCreated();

private:
	CImageList* imageList;
	void ContactDecode(CString str, CString &number, CString &name, BOOL &presence, BOOL &fromDirectory);
	void MessageDlgOpen(BOOL isCall = FALSE, BOOL hasVideo = FALSE);
	void DefaultItemAction(int i);

protected:
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnFilterValueChange();
	afx_msg void OnMenuCallPickup();
	afx_msg void OnMenuCall(); 
	afx_msg void OnMenuChat();
	afx_msg void OnMenuAdd(); 
	afx_msg void OnMenuEdit(); 
	afx_msg void OnMenuCopy(); 
	afx_msg void OnMenuDelete(); 
	afx_msg void OnMenuImportGoogle();
	afx_msg LRESULT OnContextMenu(WPARAM wParam,LPARAM lParam);
	afx_msg void OnNMDblclkContacts(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEndtrack(NMHDR* pNMHDR, LRESULT* pResult);
#ifdef _GLOBAL_VIDEO
	afx_msg void OnMenuCallVideo(); 
#endif
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

