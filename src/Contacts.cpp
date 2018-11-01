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

#include "Contacts.h"
#include "microsip.h"
#include "settings.h"
#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>
#include "mainDlg.h"
#include "utf.h"
#include "langpack.h"
#include "CSVFile.h"

Contacts::Contacts(CWnd* pParent /*=NULL*/)
: CBaseDialog(Contacts::IDD, pParent)
{
	Create (IDD, pParent);
}

Contacts::~Contacts(void)
{
}

BOOL Contacts::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	AutoMove(IDC_CONTACTS,0,0,100,100);
	AutoMove(IDC_SEARCH_PICTURE,0,100,0,0);
	AutoMove(IDC_FILER_VALUE,0,100,100,0);

	TranslateDialog(this->m_hWnd);

	addDlg = new AddDlg(this);
	imageList = new CImageList();
	imageList->Create(16,16,ILC_COLOR32,3,3);
	imageList->SetBkColor(RGB(255, 255, 255));
	imageList->Add(theApp.LoadIcon(IDI_UNKNOWN));
	imageList->Add(theApp.LoadIcon(IDI_OFFLINE));
	imageList->Add(theApp.LoadIcon(IDI_AWAY));
	imageList->Add(theApp.LoadIcon(IDI_ONLINE));
	imageList->Add(theApp.LoadIcon(IDI_ON_THE_PHONE));
	imageList->Add(theApp.LoadIcon(IDI_BLANK));
	imageList->Add(theApp.LoadIcon(IDI_BUSY));
	imageList->Add(theApp.LoadIcon(IDI_DEFAULT));

	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	list->SetImageList(imageList,LVSIL_SMALL);

	CRect rect;
	list->GetClientRect(rect);

	int defaultWidth = rect.Width() - GetSystemMetrics(SM_CXVSCROLL);
	list->InsertColumn(0, Translate(_T("Name")), LVCFMT_LEFT, accountSettings.contactsWidth0 > 0 ? accountSettings.contactsWidth0 : defaultWidth/2);
	list->InsertColumn(1, Translate(_T("Info")), LVCFMT_LEFT, accountSettings.contactsWidth1 > 0 ? accountSettings.contactsWidth1 : defaultWidth/2);
	list->SetExtendedStyle( list->GetExtendedStyle() |  LVS_EX_FULLROWSELECT );
	
	ContactsLoad();

	return TRUE;
}

void Contacts::OnCreated()
{
	m_SortItemsExListCtrl.SetSortColumn(0,true);
}

void Contacts::PostNcDestroy()
{
	CBaseDialog::PostNcDestroy();
	if (pj_ready) {
		PresenceUnsubsribe();
	}
	mainDlg->pageContacts=NULL;
	delete imageList;
	delete this;
}

void Contacts::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONTACTS, m_SortItemsExListCtrl);
}

BEGIN_MESSAGE_MAP(Contacts, CBaseDialog)
	ON_NOTIFY(HDN_ENDTRACK, 0, OnEndtrack)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_EN_CHANGE(IDC_FILER_VALUE, OnFilterValueChange)
	ON_COMMAND(ID_CALL_PICKUP,OnMenuCallPickup)
	ON_COMMAND(ID_CALL,OnMenuCall)
	ON_COMMAND(ID_CHAT,OnMenuChat)
	ON_COMMAND(ID_ADD,OnMenuAdd)
	ON_COMMAND(ID_EDIT,OnMenuEdit)
	ON_COMMAND(ID_COPY,OnMenuCopy)
	ON_COMMAND(ID_DELETE,OnMenuDelete)
	ON_COMMAND(ID_IMPORT_GOOGLE,OnMenuImportGoogle)
	ON_MESSAGE(WM_CONTEXTMENU,OnContextMenu)
	ON_NOTIFY(NM_DBLCLK, IDC_CONTACTS, &Contacts::OnNMDblclkContacts)
#ifdef _GLOBAL_VIDEO
	ON_COMMAND(ID_VIDEOCALL,OnMenuCallVideo)
#endif
END_MESSAGE_MAP()


BOOL Contacts::PreTranslateMessage(MSG* pMsg)
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

void Contacts::OnEndtrack(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *)pNMHDR;
	int width = phdn->pitem->cxy;
	switch (phdn->iItem) {
	case 0:
		accountSettings.contactsWidth0 = width;
		break;
	}
	mainDlg->AccountSettingsPendingSave();
	*pResult = 0;
}

void Contacts::OnBnClickedOk()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos) {
		DefaultItemAction(list->GetNextSelectedItem(pos));
	}
}

void Contacts::DefaultItemAction(int i)
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	Contact *pContact = (Contact *) list->GetItemData(i);
	if (pContact->ringing) {
		OnMenuCallPickup();
	} else {
		MessageDlgOpen(accountSettings.singleMode);
	}
}

void Contacts::OnBnClickedCancel()
{
	mainDlg->ShowWindow(SW_HIDE);
}

void Contacts::OnFilterValueChange()
{
	ContactsClear();
	ContactsLoad();
}

bool Contacts::isFiltered(Contact *pContact) {
	CEdit* edit = (CEdit*)GetDlgItem(IDC_FILER_VALUE);
	CString str;
	edit->GetWindowText(str);
	if (!str.IsEmpty()) {
		if (!pContact ) {
			return true;
		}
		str.MakeLower();
		CString name = pContact->name;
		CString number = pContact->number;
		name.MakeLower();
		number.MakeLower();
		if (name.Find(str) ==-1 && number.Find(str) ==-1) {
			return true;
		}
	}
	return false;
}

void Contacts::filterReset()
{
	CEdit* edit = (CEdit*)GetDlgItem(IDC_FILER_VALUE);
	edit->SetWindowText(_T(""));
}

LRESULT Contacts::OnContextMenu(WPARAM wParam,LPARAM lParam)
{
	int x = GET_X_LPARAM(lParam); 
	int y = GET_Y_LPARAM(lParam); 
	POINT pt = { x, y };
	RECT rc;
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
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
			y = 10+pt.y;
		}
	}
	if (x!=-1 || y!=-1) {
		CMenu menu;
		menu.LoadMenu(IDR_MENU_CONTACT);
		CMenu* tracker = menu.GetSubMenu(0);
		TranslateMenu(tracker->m_hMenu);
		if ( selectedItem != -1 ) {
			Contact *pContact = (Contact *) list->GetItemData(selectedItem);
			if (pContact->ringing) {
				tracker->InsertMenu(ID_CALL,0,ID_CALL_PICKUP,Translate(_T("Call Pickup")));
			}
			tracker->EnableMenuItem(ID_CALL, FALSE);
#ifdef _GLOBAL_VIDEO
			tracker->EnableMenuItem(ID_VIDEOCALL, FALSE);
#endif
			tracker->EnableMenuItem(ID_CHAT, FALSE);
			tracker->EnableMenuItem(ID_EDIT, FALSE);
			tracker->EnableMenuItem(ID_COPY, FALSE);
			tracker->EnableMenuItem(ID_DELETE, FALSE);
		} else {
			tracker->EnableMenuItem(ID_CALL, TRUE);
#ifdef _GLOBAL_VIDEO
			tracker->EnableMenuItem(ID_VIDEOCALL, TRUE);
#endif
			tracker->EnableMenuItem(ID_CHAT, TRUE);
			tracker->EnableMenuItem(ID_EDIT, TRUE);
			tracker->EnableMenuItem(ID_COPY, TRUE);
			tracker->EnableMenuItem(ID_DELETE, TRUE);
		}
		tracker->AppendMenu(0, MF_SEPARATOR);
		CMenu importMenu;
		importMenu.CreatePopupMenu();
		importMenu.AppendMenu(MF_STRING, ID_IMPORT_GOOGLE, Translate(_T("Google CSV")));
		tracker->AppendMenu(MF_POPUP, (UINT_PTR)importMenu.m_hMenu, Translate(_T("Import")));
		
		tracker->TrackPopupMenu( 0, x, y, this );
		return TRUE;
	}
	return DefWindowProc(WM_CONTEXTMENU,wParam,lParam);
}

void Contacts::MessageDlgOpen(BOOL isCall, BOOL hasVideo)
{
	if (accountSettings.singleMode && call_get_count_noincoming() && isCall) {
		mainDlg->GotoTab(0);
		return;
	}
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos) {
		int i = list->GetNextSelectedItem(pos);
		Contact *pContact = (Contact *) list->GetItemData(i);
		if (isCall) {
			mainDlg->MakeCall(pContact->number, hasVideo);
		} else {
			mainDlg->MessagesOpen(pContact->number);
		}
	}
}

void Contacts::OnNMDblclkContacts(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem!=-1) {
		DefaultItemAction(pNMItemActivate->iItem);
	}
	*pResult = 0;
}

void Contacts::OnMenuCallPickup()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos) {
		int i = list->GetNextSelectedItem(pos);
		Contact *pContact = (Contact *) list->GetItemData(i);
		mainDlg->messagesDlg->CallMake(_T(_GLOBAL_CALL_PICKUP)+pContact->number);
	}
}

void Contacts::OnMenuCall()
{
	MessageDlgOpen(TRUE);
}

#ifdef _GLOBAL_VIDEO
void Contacts::OnMenuCallVideo()
{
	MessageDlgOpen(TRUE, TRUE);
}
#endif

void Contacts::OnMenuChat()
{
	MessageDlgOpen();
}

void Contacts::OnMenuAdd()
{
	if (!addDlg->IsWindowVisible()) {
		addDlg->ShowWindow(SW_SHOW);
	} else {
		addDlg->SetForegroundWindow();
	}
	addDlg->listIndex = -1;
	addDlg->GetDlgItem(IDC_EDIT_NUMBER)->SetWindowText(_T(""));
	addDlg->GetDlgItem(IDC_EDIT_NAME)->SetWindowText(_T(""));
	((CButton *)addDlg->GetDlgItem(IDC_PRESENCE))->SetCheck(0);
	addDlg->GetDlgItem(IDC_EDIT_NAME)->SetFocus();
}

void Contacts::OnMenuEdit()
{
	OnMenuAdd();
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	int i = list->GetNextSelectedItem(pos);
	addDlg->listIndex = i;

	Contact *pContact = (Contact *) list->GetItemData(i);
	addDlg->GetDlgItem(IDC_EDIT_NUMBER)->SetWindowText(pContact->number);
	addDlg->GetDlgItem(IDC_EDIT_NAME)->SetWindowText(pContact->name);
	((CButton *)addDlg->GetDlgItem(IDC_PRESENCE))->SetCheck(pContact->presence);
}

void Contacts::OnMenuCopy()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	if (pos) {
		int i = list->GetNextSelectedItem(pos);
		Contact *pContact = (Contact *) list->GetItemData(i);
		mainDlg->CopyStringToClipboard(pContact->number);
	}
}

void Contacts::OnMenuDelete()
{
	CList<CString,CString> contacts;
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	POSITION pos = list->GetFirstSelectedItemPosition();
	while (pos)	{
		Contact *pContact = (Contact *) list->GetItemData(list->GetNextSelectedItem(pos));
		contacts.AddTail(pContact->number);
	}
	if (isFiltered()) {
		filterReset();
	}
	int count = list->GetItemCount();
	for (int i=0;i<count;i++) {
		Contact *pContact = (Contact *) list->GetItemData(i);
		if (contacts.Find(pContact->number)) {
			ContactDelete(i);
			count--;
			i--;
		}
	}
	ContactsSave();
}

void Contacts::OnMenuImportGoogle()
{
	CFileDialog dlgFile(TRUE, _T("cvs"), 0, OFN_HIDEREADONLY, _T("CSV Files (*.csv)|*.csv|"),this);
	if (dlgFile.DoModal() == IDOK) {
		bool changed = false;
		if (isFiltered()) {
			filterReset();
		}
		CCSVFile CSVFile;
		CSVFile.SetCodePage(CP_UTF8);
		if (CSVFile.Open(dlgFile.GetPathName(), CCSVFile::modeRead | CFile::typeText | CFile::shareDenyWrite)) {
			CStringArray arr;
			int nameIndex = -1, numberIndex = -1;
			while (CSVFile.ReadData(arr)) {
				if (nameIndex == -1) {
					for (int i = 0; i < arr.GetCount(); i++) {
						CString s = arr.GetAt(i);
						if (nameIndex == -1 && arr.GetAt(i).CompareNoCase(_T("Name")) == 0) {
							nameIndex = i;
						}
						if (numberIndex == -1 && arr.GetAt(i).CompareNoCase(_T("Phone 1 - Value")) == 0) {
							numberIndex = i;
						}
					}
					if (nameIndex == -1 || numberIndex == -1) {
						AfxMessageBox(_T("Unknown format"));
						break;
					}
				}
				else if (arr.GetCount() > numberIndex && arr.GetCount() > nameIndex) {
					CString number = arr.GetAt(numberIndex);
					CString name = arr.GetAt(nameIndex);
					if (!number.IsEmpty() && ContactAdd(number, name, 0, 0, FALSE, TRUE) && !changed) {
						changed = true;
					}
				}
			}
			CSVFile.Close();
		}
		if (changed) {
			ContactsSave();
		}
	}
}

void Contacts::ContactDelete(int i)
{
	int deleted = 0;
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	Contact *pContact = (Contact *) list->GetItemData(i);
	PresenceUnsubsribeOne(pContact);
	list->DeleteItem(i);
	delete pContact;
}

bool Contacts::ContactAdd(CString number, CString name, char presence, char directory, BOOL save, BOOL fromDirectory)
{
	if (save) {
		if (isFiltered()) {
			filterReset();
		}
	}
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	if (fromDirectory || save) {
		int count = list->GetItemCount();
		for (int i=0;i<count;i++) {
			Contact *pContact = (Contact *) list->GetItemData(i);
			CString contactData;
			if (pContact->number == number) {
				pContact->candidate = FALSE;
				bool changed = false;
				if (!name.IsEmpty() && name!=pContact->name) {
					list->SetItemText(i,0,name);
					pContact->name=name;
					changed = true;
				}
				if (presence!=-1 && presence!=pContact->presence) {
					pContact->presence=presence;
					if (presence>0) {
						PresenceSubsribeOne(pContact);
					}
					changed = true;
				}
				if (!fromDirectory && directory!=-1 && directory!=pContact->directory) {
					pContact->directory=directory;
					changed = true;
				}
				if (save && changed) {
					ContactsSave();
				}
				return changed;
			}
		}
	}
	if (name.IsEmpty()) {
		name = number;
	}

	Contact *pContact;
	bool found = false;
	if (pj_ready && isSubscribed) {
		pjsua_buddy_id ids[PJSUA_MAX_BUDDIES];
		unsigned count = PJSUA_MAX_BUDDIES;
		pjsua_enum_buddies(ids,&count);
		for (unsigned i=0;i<count;i++) {
			pContact =  (Contact *)pjsua_buddy_get_user_data(ids[i]);
			if (pContact && pContact->number == number) {
				found = true;
				break;
			}
		}
	}
	if (!found) {
		pContact =  new Contact();
		pContact->image = 7;
	}
	pContact->number = number;
	pContact->name = name;
	pContact->presence = presence>0;
	pContact->directory = directory>0;
	int i = list->InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,0,name,0,0,pContact->image,(LPARAM)pContact);
	//list->SetItemText(i,1,number);

	if (save) {
		ContactsSave();
	}
	if (save || fromDirectory) {
		PresenceSubsribeOne(pContact);
	}
	return true;
}

void Contacts::ContactsSave()
{
	if (isFiltered()) {
		filterReset();
	}
	CXMLFile xmlFile; 
	CXMLElement* xmlRoot = new CXMLElement();
	xmlRoot->Create(_T("XML:ROOT"), XET_TAG);
	CXMLElement* xmlContacts = new CXMLElement();
	xmlContacts->Create(_T("contacts"), XET_TAG);
	xmlRoot->AppendChild(xmlContacts);
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	int count = list->GetItemCount();
	CXMLElement* xmlContact;
	CXMLElement* xmlAttr;
	for (int i=0;i<count;i++) {
		Contact *pContact = (Contact *) list->GetItemData(i);
		xmlContact = new CXMLElement();
		xmlContact->Create(_T("contact"), XET_TAG);
		xmlAttr = new CXMLElement();
		xmlAttr->Create(_T("number"), XET_ATTRIBUTE);
		xmlAttr->SetValue(AnsiToUnicode(Utf8EncodeUcs2(XMLEntityEncode(pContact->number))).GetBuffer());
		xmlContact->AppendChild(xmlAttr);
		xmlAttr = new CXMLElement();
		xmlAttr->Create(_T("name"), XET_ATTRIBUTE);
		xmlAttr->SetValue(AnsiToUnicode(Utf8EncodeUcs2(XMLEntityEncode(pContact->name))).GetBuffer());
		xmlContact->AppendChild(xmlAttr);
		xmlAttr = new CXMLElement();
		xmlAttr->Create(_T("presence"), XET_ATTRIBUTE);
		xmlAttr->SetValue(pContact->presence?_T("1"):_T("0"));
		xmlContact->AppendChild(xmlAttr);
		xmlAttr = new CXMLElement();
		xmlAttr->Create(_T("directory"), XET_ATTRIBUTE);
		xmlAttr->SetValue(pContact->directory?_T("1"):_T("0"));
		xmlContact->AppendChild(xmlAttr);
		xmlContacts->AppendChild(xmlContact);
	}
	xmlFile.SetRoot(xmlRoot);
	CString filename = accountSettings.pathRoaming;
	filename.Append(_T("Contacts.xml"));
	xmlFile.SaveToFile(filename.GetBuffer());
}

void Contacts::ContactsClear()
{
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	list->DeleteAllItems();
}

void Contacts::ContactsLoad()
{
	CXMLFile xmlFile;
	CString filename = accountSettings.pathRoaming;
	filename.Append(_T("Contacts.xml"));
	if (xmlFile.LoadFromFile(filename.GetBuffer())) {
		CXMLElement *xmlRoot = xmlFile.GetRoot();
		CXMLElement *xmlContacts = xmlRoot->GetFirstChild();
		while (xmlContacts) {
			if (xmlContacts->GetElementType() == XET_TAG) {
				CXMLElement *xmlContact = xmlContacts->GetFirstChild();
				while (xmlContact) {
					if (xmlContact->GetElementType() == XET_TAG) {
						CXMLElement *xmlAttr = xmlContact->GetFirstChild();
						CString number;
						CString name;
						BOOL presence = FALSE;
						BOOL directory = FALSE;
						CString rab;
						while (xmlAttr) {
							if (xmlAttr->GetElementType() == XET_ATTRIBUTE) {
								CString attrName = xmlAttr->GetElementName();
								if (attrName == _T("number")) {
									number = XMLEntityDecode(Utf8DecodeUni(UnicodeToAnsi(xmlAttr->GetValue())));
								} else if (attrName == _T("name")) {
									name = XMLEntityDecode(Utf8DecodeUni(UnicodeToAnsi(xmlAttr->GetValue())));
								} else if (attrName == _T("presence")) {
									rab = xmlAttr->GetValue();
									presence = rab==_T("1");
								} else if (attrName == _T("directory")) {
									rab = xmlAttr->GetValue();
									directory = rab==_T("1");
								}
							}
							xmlAttr = xmlContact->GetNextChild();
						}
						if (!number.IsEmpty()) {
							Contact contact;
							contact.name = name;
							contact.number = number;
							if (!isFiltered(&contact)) {
								ContactAdd(number, name, presence, directory, FALSE);
							}
						}
					}
					xmlContact = xmlContacts->GetNextChild();
				}
			}
			xmlContacts = xmlRoot->GetNextChild();
		}
	} else {
		CString key;
		CString val;
		LPTSTR ptr = val.GetBuffer(255);
		int i=0;
		while (TRUE) {
			key.Format(_T("%d"),i);
			if (GetPrivateProfileString(_T("Contacts"), key, NULL, ptr, 256, accountSettings.iniFile)) {
				CString number;
				CString name;
				BOOL presence;
				BOOL directory;
				ContactDecode(ptr, number, name, presence, directory);
				ContactAdd(number, name, presence, directory, FALSE);
			} else {
				break;
			}
			i++;
		}
		WritePrivateProfileSection(_T("Contacts"),  NULL, accountSettings.iniFile);
		ContactsSave();
	}
	m_SortItemsExListCtrl.SortColumn(m_SortItemsExListCtrl.GetSortColumn(),m_SortItemsExListCtrl.IsAscending());
}

void Contacts::ContactDecode(CString str, CString &number, CString &name, BOOL &presence, BOOL &fromDirectory)
{
	CString rab;
	int begin;
	int end;
	begin = 0;
	end = str.Find(';', begin);
	if (end != -1)
	{
		number=str.Mid(begin, end-begin);
		begin = end + 1;
		end = str.Find(';', begin);
		if (end != -1)
		{
			name=str.Mid(begin, end-begin);
			begin = end + 1;
			end = str.Find(';', begin);
			if (end != -1)
			{
				rab=str.Mid(begin, end-begin);
				presence = rab == _T("1");
				begin = end + 1;
				end = str.Find(';', begin);
				if (end != -1)
				{
					rab=str.Mid(begin, end-begin);
					fromDirectory = rab == _T("1");
				} else 
				{
					rab=str.Mid(begin);
					fromDirectory = rab == _T("1");
				}
			} else 
			{
				rab=str.Mid(begin);
				presence = rab == _T("1");
				fromDirectory = FALSE;
			}
		} else 
		{
			name = str.Mid(begin);
			presence = FALSE;
			fromDirectory = FALSE;
		}
	} else 
	{
		number=str;
		name = number;
		presence = FALSE;
		fromDirectory = FALSE;
	}
}

CString Contacts::GetNameByNumber(CString number)
{
	if (isFiltered()) {
		filterReset();
	}

	CString name;
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);

	CString sipURI = GetSIPURI(number);
	int n = list->GetItemCount();
	for (int i=0; i<n; i++) {
		Contact* pContact = (Contact *) list->GetItemData(i);
		if (GetSIPURI(pContact->number) == sipURI)
		{
			name = pContact->name;
			break;
		}
	}
	return name;
}

void Contacts::PresenceSubsribeOne(Contact *pContact)
{
	if (isSubscribed && pContact->presence) {
		pjsua_buddy_id p_buddy_id;
		pjsua_buddy_config buddy_cfg;
		pjsua_buddy_config_default(&buddy_cfg);
		buddy_cfg.subscribe=PJ_TRUE;
		buddy_cfg.uri = StrToPjStr(GetSIPURI(pContact->number));
		buddy_cfg.user_data = (void *)pContact;
		pj_status_t status = pjsua_buddy_add(&buddy_cfg, &p_buddy_id);
		if (status != PJ_SUCCESS) {
			AfxMessageBox(_T("Presence Subscription error: ")+GetErrorMessage(status));
		}
	}
}

void Contacts::PresenceUnsubsribeOne(Contact *pContact)
{
	if (isSubscribed)
	{
		pjsua_buddy_id ids[PJSUA_MAX_BUDDIES];
		unsigned count = PJSUA_MAX_BUDDIES;
		pjsua_enum_buddies(ids,&count);
		for (unsigned i=0;i<count;i++)
		{
			if ((Contact *)pjsua_buddy_get_user_data(ids[i])==pContact)
			{
				pjsua_buddy_del(ids[i]);
				break;
			}
		}
	}
}

void Contacts::PresenceSubsribe()
{
	if (!isSubscribed) {
		if (isFiltered()) {
			filterReset();
		}
		isSubscribed=TRUE;
		CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
		int n = list->GetItemCount();
		for (int i=0; i<n; i++) {
			Contact *pContact = (Contact *) list->GetItemData(i);
			PresenceSubsribeOne(pContact);
		}
	}
}

void Contacts::PresenceUnsubsribe()
{
	pjsua_buddy_id ids[PJSUA_MAX_BUDDIES];
	unsigned count = PJSUA_MAX_BUDDIES;
	pjsua_enum_buddies(ids,&count);
	for (unsigned i=0;i<count;i++)
	{
		pjsua_buddy_del(ids[i]);
	}
	if (::IsWindow(this->m_hWnd)) {
		CListCtrl *list= (CListCtrl *)GetDlgItem(IDC_CONTACTS);
		int n = list->GetItemCount();
		for (int i=0; i<n; i++)
		{
			list->SetItem(i, 0, LVIF_IMAGE, 0, 7, 0, 0, 0);
		}
	}
	isSubscribed=FALSE;
}

void Contacts::SetCanditates()
{
	if (isFiltered()) {
		filterReset();
	}
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	int count = list->GetItemCount();
	for (int i=0;i<count;i++)
	{
		Contact *pContact = (Contact *) list->GetItemData(i);
		if (pContact->directory) {
			pContact->candidate = TRUE;
		}
	}
}
int Contacts::DeleteCanditates()
{
	if (isFiltered()) {
		filterReset();
	}
	CListCtrl *list= (CListCtrl*)GetDlgItem(IDC_CONTACTS);
	int count = list->GetItemCount();
	int deleted = 0;
	for (int i=0;i<count;i++)
	{
		Contact *pContact = (Contact *) list->GetItemData(i);
		if (pContact->candidate) {
			ContactDelete(i);
			count--;
			i--;
			deleted++;
		}
	}
	return deleted;
}


