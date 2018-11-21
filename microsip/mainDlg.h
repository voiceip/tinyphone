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

#include "stdafx.h"

#include <afxwin.h>

#include "define.h"
#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>

#ifndef _WIN64
#ifdef NDEBUG
#ifdef _GLOBAL_VIDEO
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Release-Static-Video.lib")
#else
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Release-Static-NoVideo.lib")
#endif
#else
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Debug-Static.lib")
#endif
#else
#ifdef NDEBUG
#ifdef _GLOBAL_VIDEO
#pragma comment(lib, "libpjproject-x86_64-x64-vc14-Release-Static-Video.lib")
#else
#pragma comment(lib, "libpjproject-x86_64-x64-vc14-Release-Static-NoVideo.lib")
#endif
#else
#pragma comment(lib, "libpjproject-x86_64-x64-vc14-Debug-Static.lib")
#endif
#endif

#include "MMNotificationClient.h"

#include "BaseDialog.h"
#include "RinginDlg.h"
#include "AccountDlg.h"
#include "SettingsDlg.h"
#include "ShortcutsDlg.h"
#include "MessagesDlg.h"

#include "Dialer.h"
#include "Contacts.h"
#include "Calls.h"
#include "Preview.h"
#include "Transfer.h"
#include "addons.h"

// CmainDlg dialog
class CmainDlg : public CBaseDialog
{
	// Construction
public:
	CmainDlg(CWnd* pParent = NULL);	// standard constructor
	~CmainDlg();

	// Dialog Data
	enum { IDD = IDD_MAIN };

	bool m_startMinimized;
	CPoint windowSize;
	CButton m_ButtonMenu;
	SettingsDlg* settingsDlg;
	bool shortcutsEnabled;
	bool shortcutsBottom;
	ShortcutsDlg* shortcutsDlg;
	MessagesDlg* messagesDlg;
	Transfer* transferDlg;
	AccountDlg* accountDlg;

	Dialer* pageDialer;
	Contacts* pageContacts;
	Calls* pageCalls;

	BOOL notStopRinging;
	CArray <RinginDlg*> ringinDlgs;
	CString dialNumberDelayed;
	pjsua_call_id autoAnswerCallId;
	pjsua_acc_config acc_cfg;

	pjsua_transport_id transport_udp_local;
	pjsua_transport_id transport_udp;
	pjsua_transport_id transport_tcp;
	pjsua_transport_id transport_tls;
	pjsua_player_id player_id;

	int iconStatusbar;
	int widthAdd;
	int heightAdd;
	bool missed;

	CString callIdIncomingIgnore;
	CList<int,int> toneCalls;
	CList<int,int> attendedCalls;
	CList<CString> audioCodecList;
	CList<int> confernceCalls;
	
	void OnCreated();
	void PJCreate();
	void PJDestroy();
	void PJAccountAdd();
	void PJAccountAddLocal();
	void PJAccountDelete(bool deep = false);
	void PJAccountDeleteLocal();
	void PJAccountConfig(pjsua_acc_config *acc_cfg);

	void CommandLine(CString params);
	void TabFocusSet();
	void UpdateWindowText(CString = CString(), int icon = IDI_DEFAULT, bool afterRegister = false);
	void PublishStatus(bool online = true, bool init=false);
	void BaloonPopup(CString title, CString message, DWORD flags = NIIF_WARNING);
	bool GotoTabLParam(LPARAM lParam);
	bool GotoTab(int i, CTabCtrl* tab = NULL);
	void DialNumberFromCommandLine(CString number);
	void DialNumber(CString params);
	bool MakeCall(CString number, bool hasVideo = false);
	bool MessagesOpen(CString number);
	void AutoAnswer(pjsua_call_id call_id);
	void ShortcutAction(Shortcut *shortcut);
	void PlayerPlay(CString filename, bool noLoop = false, bool inCall = false);
	BOOL CopyStringToClipboard( IN const CString & str );
	void OnTimerCall ();

	void UsersDirectoryLoad();
	void OnTimerContactBlink();
	afx_msg LRESULT onUsersDirectoryLoaded(WPARAM wParam,LPARAM lParam);
	void SetupJumpList();
	void RemoveJumpList();
	void MainPopupMenu();
	void SetPaneText2(CString str = _T(""));
	void AccountSettingsPendingSave();
	void UpdateSoundDevicesIds();
	void PlayerStop();
	
#ifdef _GLOBAL_VIDEO
	Preview* previewWin;
	int VideoCaptureDeviceId(CString name=_T(""));
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Implementation
protected:
	HICON m_hIcon;
	HICON iconSmall;
	HICON iconInactive;
	HICON iconMissed;
	NOTIFYICONDATA tnd;
	CStatusBar m_bar;

	CMMNotificationClient *mmNotificationClient;

	unsigned char m_tabPrev;

	DWORD m_lastInputTime;
	int m_idleCounter;
	pjrpid_activity m_PresenceStatus;
	bool newMessages;
		
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// Generated message map functions
	afx_msg LRESULT OnUpdateWindowText(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onTrayNotify(WPARAM, LPARAM);
	afx_msg LRESULT onCreateRingingDlg(WPARAM, LPARAM);
	afx_msg LRESULT onRefreshLevels(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onRegState2(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onCallState(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onMWIInfo(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onCallMediaState(WPARAM, LPARAM);
	afx_msg LRESULT onCallTransferStatus(WPARAM, LPARAM);
	afx_msg LRESULT onPager(WPARAM, LPARAM);
	afx_msg LRESULT onPagerStatus(WPARAM, LPARAM);
	afx_msg LRESULT onBuddyState(WPARAM, LPARAM);
	afx_msg LRESULT onCopyData(WPARAM, LPARAM);
	afx_msg LRESULT CreationComplete(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT onPowerBroadcast(WPARAM, LPARAM);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedMenu();
	afx_msg void OnClose();
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint point );
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	afx_msg void OnSessionChange(UINT nSessionState, UINT nId);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT type, int w, int h);
	afx_msg LRESULT onShellHookMessage(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onCallAnswer(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onCallHangup(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onTabIconUpdate(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onSetPaneText(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onPlayerPlay(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT onPlayerStop(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnAccount(WPARAM wParam,LPARAM lParam);
	afx_msg void OnMenuAccountAdd();
	afx_msg void OnMenuAccountChange(UINT nID);
	afx_msg void OnMenuAccountEdit(UINT nID);
	afx_msg void OnMenuCustomRange(UINT nID);
	afx_msg void OnMenuSettings();
	afx_msg void OnMenuShortcuts();
	afx_msg void OnMenuAlwaysOnTop();
	afx_msg void OnMenuLog();
	afx_msg void OnMenuExit();
	afx_msg void OnTimer (UINT_PTR TimerVal);
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangingTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuWebsite();
	afx_msg void OnMenuHelp();
	afx_msg void OnMenuAddl();
	afx_msg void CheckUpdates();
#ifdef _GLOBAL_VIDEO
	afx_msg void createPreviewWin();
#endif
};

extern CmainDlg *mainDlg;
