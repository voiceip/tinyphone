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
#include "Calls.h"
#include "microsip.h"
#include "global.h"
#include "settings.h"
#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>
#include "mainDlg.h"
#include "langpack.h"

Calls::Calls(CWnd* pParent /*=NULL*/)
: CBaseDialog(Calls::IDD, pParent)
{
	Create (IDD, pParent);
}

Calls::~Calls(void)
{
}

BOOL Calls::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	AutoMove(IDC_CALLS,0,0,100,100);
	AutoMove(IDC_SEARCH_PICTURE,0,100,0,0);
	AutoMove(IDC_FILER_VALUE,0,100,100,0);

	TranslateDialog(this->m_hWnd);

	nextKey = 0;
	lastDay = 0;
	
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);

	list->SetExtendedStyle( list->GetExtendedStyle() |  LVS_EX_FULLROWSELECT );

	imageList = new CImageList();
	imageList->Create(16,16,ILC_COLOR32,3,3);
	imageList->SetBkColor(RGB(255, 255, 255));
	imageList->Add(theApp.LoadIcon(IDI_CALL_OUT));
	imageList->Add(theApp.LoadIcon(IDI_CALL_IN));
	imageList->Add(theApp.LoadIcon(IDI_CALL_MISS));
	list->SetImageList(imageList,LVSIL_SMALL);
	list->InsertColumn(0,Translate(_T("Number")),LVCFMT_LEFT, accountSettings.callsWidth0>0?accountSettings.callsWidth0:81);
	list->InsertColumn(1,Translate(_T("Time")),LVCFMT_LEFT, accountSettings.callsWidth1>0?accountSettings.callsWidth1:71);
	list->InsertColumn(2,Translate(_T("Duration")),LVCFMT_LEFT, accountSettings.callsWidth2>0?accountSettings.callsWidth2:40);
	list->InsertColumn(3,Translate(_T("Info")),LVCFMT_LEFT, accountSettings.callsWidth3>0?accountSettings.callsWidth3:50);

	CallsLoad();

	return TRUE;
}

void Calls::OnCreated()
{
	m_SortItemsExListCtrl.SetSortColumn(1,false);
}

void Calls::PostNcDestroy()
{
	CBaseDialog::PostNcDestroy();
	mainDlg->pageCalls=NULL;
	delete imageList;
	delete this;
}

void Calls::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CALLS, m_SortItemsExListCtrl);
}


BEGIN_MESSAGE_MAP(Calls, CBaseDialog)
	ON_WM_CREATE()
	ON_NOTIFY(HDN_ENDTRACK, 0, OnEndtrack)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_EN_CHANGE(IDC_FILER_VALUE, OnFilterValueChange)
	ON_COMMAND(ID_CALL,OnMenuCall)
	ON_COMMAND(ID_CHAT,OnMenuChat)
	ON_COMMAND(ID_COPY,OnMenuCopy)
	ON_COMMAND(ID_DELETE,OnMenuDelete)
	ON_NOTIFY(NM_DBLCLK, IDC_CALLS, &Calls::OnNMDblclkCalls)
	ON_MESSAGE(WM_CONTEXTMENU,OnContextMenu)
#ifdef _GLOBAL_VIDEO
	ON_COMMAND(ID_VIDEOCALL,OnMenuCallVideo)
#endif
END_MESSAGE_MAP()

BOOL Calls::PreTranslateMessage(MSG* pMsg)
{
	BOOL catched = FALSE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
		CEdit* edit = (CEdit*)GetDlgItem(IDC_FILER_VALUE);
		if (edit == GetFocus()) {
			catched = TRUE;
			if (isFiltered()) {
				filterReset();
			}
		}
	}
	if (!catched) {
		return CBaseDialog::PreTranslateMessage(pMsg);
	} else {
		return TRUE;
	}
}

void Calls::OnEndtrack(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *)pNMHDR;
	int width = phdn->pitem->cxy;
	switch (phdn->iItem) {
	case 0:
		accountSettings.callsWidth0 = width;
		break;
	case 1:
		accountSettings.callsWidth1 = width;
		break;
	case 2:
		accountSettings.callsWidth2 = width;
		break;
	case 3:
		accountSettings.callsWidth3 = width;
		break;
	case 4:
		accountSettings.callsWidth4 = width;
		break;
	}
	mainDlg->AccountSettingsPendingSave();
	*pResult = 0;
}


void Calls::OnBnClickedOk()
{
	MessageDlgOpen(accountSettings.singleMode);
}

void Calls::OnBnClickedCancel()
{
	mainDlg->ShowWindow(SW_HIDE);
}

void Calls::OnFilterValueChange()
{
	CallsClear();
	CallsLoad();
}

bool Calls::isFiltered(Call *pCall) {
	CEdit* edit = (CEdit*)GetDlgItem(IDC_FILER_VALUE);
	CString str;
	edit->GetWindowText(str);
	if (!str.IsEmpty()) {
		if (!pCall ) {
			return true;
		}
		str.MakeLower();
		CString name = pCall->name;
		CString number = pCall->number;
		name.MakeLower();
		number.MakeLower();
		if (name.Find(str) ==-1 && number.Find(str) ==-1) {
			return true;
		}
	}
	return false;
}

void Calls::filterReset()
{
	CEdit* edit = (CEdit*)GetDlgItem(IDC_FILER_VALUE);
	edit->SetWindowText(_T(""));
}

LRESULT Calls::OnContextMenu(WPARAM wParam,LPARAM lParam)
{
	int x = GET_X_LPARAM(lParam); 
	int y = GET_Y_LPARAM(lParam); 
	POINT pt = { x, y };
	RECT rc;
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	int selectedItem = -1;
	if (pos) {
		selectedItem = list->GetNextSelectedItem(pos);
	}
	if (x!=-1 || y!=-1) {
		ScreenToClient(&pt);
		GetClientRect(&rc); 
		if (!PtInRect(&rc, pt)) {
			x = y = -1;
		} 
	} else {
		if (selectedItem != -1) {
			list->GetItemPosition(selectedItem,&pt);
			list->ClientToScreen(&pt);
			x = 40+pt.x;
			y = 8+pt.y;
		} else {
			::ClientToScreen((HWND)wParam, &pt);
			x = 10+pt.x;
			y = 26+pt.y;
		}
	}
	if (x!=-1 || y!=-1) {
		CMenu menu;
		if (menu.LoadMenu(IDR_MENU_CONTACT)) {
			CMenu* tracker = menu.GetSubMenu(0);
			TranslateMenu(tracker->m_hMenu);
			tracker->RemoveMenu(ID_ADD,0);
			tracker->RemoveMenu(ID_EDIT,0);
			if (selectedItem != -1) {
				tracker->EnableMenuItem(ID_CALL, FALSE);
#ifdef _GLOBAL_VIDEO
				tracker->EnableMenuItem(ID_VIDEOCALL, FALSE);
#endif
				tracker->EnableMenuItem(ID_CHAT, FALSE);
				tracker->EnableMenuItem(ID_COPY, FALSE);
				tracker->EnableMenuItem(ID_DELETE, FALSE);
			} else {
				tracker->EnableMenuItem(ID_CALL, TRUE);
#ifdef _GLOBAL_VIDEO
				tracker->EnableMenuItem(ID_VIDEOCALL, TRUE);
#endif
				tracker->EnableMenuItem(ID_CHAT, TRUE);
				tracker->EnableMenuItem(ID_COPY, TRUE);
				tracker->EnableMenuItem(ID_DELETE, TRUE);
			}
			tracker->TrackPopupMenu( 0, x, y, this );
		}
		return TRUE;
	}
	return DefWindowProc(WM_CONTEXTMENU,wParam,lParam);
}

void Calls::MessageDlgOpen(BOOL isCall, BOOL hasVideo)
{
	if (accountSettings.singleMode && call_get_count_noincoming() && isCall) {
		mainDlg->GotoTab(0);
		return;
	}
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos) {
		int i = list->GetNextSelectedItem(pos);
		Call *pCall = (Call *) list->GetItemData(i);
		if (isCall) {
			mainDlg->MakeCall(pCall->number, hasVideo);
		} else {
			mainDlg->MessagesOpen(pCall->number);
		}
	}
}

void Calls::OnNMDblclkCalls(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem!=-1) {
		MessageDlgOpen(accountSettings.singleMode);
	}
	*pResult = 0;
}

void Calls::OnMenuCall()
{
	MessageDlgOpen(TRUE);
}

#ifdef _GLOBAL_VIDEO
void Calls::OnMenuCallVideo()
{
	MessageDlgOpen(TRUE, TRUE);
}
#endif

void Calls::OnMenuChat()
{
	MessageDlgOpen();
}

void Calls::OnMenuCopy()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos)
	{
		int i = list->GetNextSelectedItem(pos);
		Call *pCall = (Call *) list->GetItemData(i);
		mainDlg->CopyStringToClipboard(pCall->number);
	}
}

void Calls::OnMenuDelete()
{
	CListCtrl *pList= (CListCtrl*)GetDlgItem(IDC_CALLS);
	POSITION pos = pList->GetFirstSelectedItemPosition();
	while (pos)	{
		Delete(pList->GetNextSelectedItem(pos));
		pos = pList->GetFirstSelectedItemPosition();
	}
}

void Calls::Delete(int i)
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	Call *pCall = (Call *) list->GetItemData(i);
	pCall->number = _T("");
	CallSave(pCall);
	delete pCall;
	list->DeleteItem(i);
}

void Calls::Add(pj_str_t id, CString number, CString name, int type)
{
	CString callId = PjToStr(&id);
	int i = Get(callId);
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	if (i==-1) {
		ReloadTime();
		SIPURI sipuri;
		ParseSIPURI(number, &sipuri);
		if (!sipuri.user.IsEmpty() || !sipuri.domain.IsEmpty()) {
			Call *pCall =  new Call();
			pCall->id = callId;
			if (sipuri.user.IsEmpty()) {
				pCall->number = sipuri.domain+sipuri.parameters;
			} else {
				if (sipuri.parameters.IsEmpty() && (accountSettings.account.domain == sipuri.domain || sipuri.domain.IsEmpty())) {
					pCall->number = sipuri.user;
				} else {
					pCall->number = sipuri.user+_T("@")+sipuri.domain+sipuri.parameters;
				}
			}
			pCall->name = name;
			pCall->type = type;
			pCall->time = CTime::GetCurrentTime().GetTime();
			pCall->duration = 0;
			if (nextKey>=1000) {
				nextKey = 0;
			}
			pCall->key = nextKey;
			Insert(pCall);
			CallSave(pCall);
		}
	} else {
		Call *pCall = (Call *) list->GetItemData(i);
		if (pCall->type == MSIP_CALL_MISS && pCall->type != type) {
			pCall->type = type;
			list->SetItem(i,0,LVIF_IMAGE,NULL,type,0,0,0);
			CallSave(pCall);
		}
	}
}

void Calls::SetDuration(pj_str_t id, int sec) {
	CString callId = PjToStr(&id);
	int i = Get(callId);
	if (i!=-1) {
		CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
		Call *pCall = (Call *) list->GetItemData(i);
		pCall->duration = sec;
		list->SetItemText(i,2,GetDuration(pCall->duration));
		CallSave(pCall);
	}
}

void Calls::SetInfo(pj_str_t id, CString str) {
	CString callId = PjToStr(&id);
	int i = Get(callId);
	if (i!=-1) {
		CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
		Call *pCall = (Call *) list->GetItemData(i);
		pCall->info = str;
		list->SetItemText(i,3,str);
		CallSave(pCall);
	}
}

int Calls::Get(CString id)
{
	if (isFiltered()) {
		filterReset();
	}
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	int count = list->GetItemCount();
	for (int i=0;i<count;i++)
	{
		Call *pCall = (Call *) list->GetItemData(i);
		if (pCall->id == id) {
			return i;
		}
	}
	return -1;
}


void Calls::Insert(Call *pCall, int pos)
{
	if (isFiltered(pCall)) {
		return;
	}
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	CString number;
	if (pCall->name != pCall->number) {
		number.AppendFormat(_T("%s (%s)"),pCall->name,pCall->number);
	} else {
		number = pCall->name;
	}
	int i = list->InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,pos,number,0,0,pCall->type,(LPARAM)pCall);
	list->SetItemText(i,1,FormatTime(pCall->time));
	list->SetItemText(i,2,GetDuration(pCall->duration));
	list->SetItemText(i,3,pCall->info);
}

void Calls::CallsClear()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
	int count = list->GetItemCount();
	for (int i=0;i<count;i++) {
		Call *pCall = (Call *) list->GetItemData(i);
		delete pCall;
	}
	list->DeleteAllItems();
}

CString Calls::FormatTime(int time, CTime *pTimeNow)
{
	CTime timeNow;
	if (!pTimeNow) {
		timeNow = CTime::GetCurrentTime();   
		pTimeNow = &timeNow;
	}
	if (!lastDay) {
		lastDay = pTimeNow->GetDay();
	}
	CTime timeCall(time);
	return timeCall.Format(
			pTimeNow->GetYear()==timeCall.GetYear() &&
			pTimeNow->GetMonth()==timeCall.GetMonth() &&
			pTimeNow->GetDay()==timeCall.GetDay()
			?_T("%X"):_T("%c")
			);
}

void Calls::ReloadTime()
{
	CTime timeNow = CTime::GetCurrentTime();   
	if (lastDay && lastDay != timeNow.GetDay()) {
		lastDay = timeNow.GetDay();
		CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);
		int count = list->GetItemCount();
		for (int i=0;i<count;i++) {
			Call *pCall = (Call *) list->GetItemData(i);
			CTime timeCall(pCall->time);
			list->SetItemText(i,1,FormatTime(pCall->time, &timeNow));
		}
	}
}


void Calls::CallSave(Call *pCall)
{
	CString key;
	// pCall->number == "" means delete
	CString data = !pCall->number.IsEmpty() ? CallEncode(pCall) : _T("null");
	key.Format(_T("%d"), pCall->key);
	WritePrivateProfileString(_T("Calls"), key, data, accountSettings.iniFile);
	if (pCall->key == nextKey) {
		nextKey++;
	}
}

void Calls::CallsLoad()
{
	CString key;
	CString val;
	LPTSTR ptr = val.GetBuffer(255);
	int prevTime = 0;
	int pos = -1;
	int inserted = 0;
	nextKey=0;
	int i=0;
	while (true) {
		key.Format(_T("%d"),i);
		if (GetPrivateProfileString(_T("Calls"), key, NULL, ptr, 256, accountSettings.iniFile)) {
			if (val != _T("null")) {
				Call *pCall =  new Call();
				CallDecode(ptr, pCall);
				bool skip = false;
				if (isFiltered(pCall)) {
					skip = true;
					delete pCall;
				}
				if (!skip) {
					pCall->key = i;
					if (pos == -1) {
						if ( prevTime > pCall->time) {
							pos = inserted;
						}
					}
					if (pos == -1) {
						Insert(pCall);
						prevTime = pCall->time;
						nextKey = pCall->key;
					} else {
						Insert(pCall, pos);
					}
					inserted++;
				}
			}
		} else {
			i--;
			break;
		}
		i++;
	}
	if (i!=-1) {
		nextKey++;
	}
	m_SortItemsExListCtrl.SortColumn(m_SortItemsExListCtrl.GetSortColumn(),m_SortItemsExListCtrl.IsAscending());
}

CString Calls::CallEncode(Call *pCall)
{
	CString data;
	data.Format(_T("%s;%s;%d;%d;%d;%s"), pCall->number, pCall->name, pCall->type, pCall->time, pCall->duration, pCall->info);
	return data;
}

void Calls::CallDecode(CString str, Call *pCall)
{
	pCall->number=str;
	pCall->name = pCall->number;
	pCall->type = 0;
	pCall->time = 0;
	pCall->duration = 0;

	CString rab;
	int begin;
	int end;
	begin = 0;
	end = str.Find(';', begin);

	if (end != -1)
	{
		pCall->number=str.Mid(begin, end-begin);
		begin = end + 1;
		end = str.Find(';', begin);
		if (end != -1)
		{
			pCall->name=str.Mid(begin, end-begin);
			begin = end + 1;
			end = str.Find(';', begin);
			if (end != -1)
			{
				pCall->type=atoi(CStringA(str.Mid(begin, end-begin)));
				if (pCall->type>2 || pCall->type<0) {
					pCall->type = 0;
				}
				begin = end + 1;
				end = str.Find(';', begin);
				if (end != -1)
				{
					pCall->time=atoi(CStringA(str.Mid(begin, end-begin)));
					begin = end + 1;
					end = str.Find(';', begin);
					if (end != -1)
					{
						pCall->duration=atoi(CStringA(str.Mid(begin, end-begin)));
						begin = end + 1;
						end = str.Find(';', begin);
						if (end != -1)
						{
							pCall->info=str.Mid(begin, end-begin);
							begin = end + 1;
							end = str.Find(';', begin);
						} else {
							pCall->info=str.Mid(begin);
						}
					}
				}
			}
		}
	}
}
/*
CString Calls::GetNameByNumber(CString number)
{
	CString name;
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CALLS);

	CString sipURI = GetSIPURI(number);
	int n = list->GetItemCount();
	for (int i=0; i<n; i++) {
		Call* pCall = (Call *) list->GetItemData(i);
		if (GetSIPURI(pCall->number) == sipURI)
		{
			name = pCall->name;
			break;
		}
	}
	return name;
}
*/
