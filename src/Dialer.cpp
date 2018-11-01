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
#include "Dialer.h"
#include "global.h"
#include "settings.h"
#include "mainDlg.h"
#include "microsip.h"
#include "Strsafe.h"
#include "langpack.h"

Dialer::Dialer(CWnd* pParent /*=NULL*/)
	: CBaseDialog(Dialer::IDD, pParent)
{
	Create(IDD, pParent);
}

Dialer::~Dialer(void)
{
}

void Dialer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DIALER_VOICEMAIL, m_ButtonVoicemail);
	DDX_Control(pDX, IDC_VOLUME_INPUT, m_SliderCtrlInput);
	DDX_Control(pDX, IDC_VOLUME_OUTPUT, m_SliderCtrlOutput);
	DDX_Control(pDX, IDC_KEY_1, m_ButtonDialer1);
	DDX_Control(pDX, IDC_KEY_2, m_ButtonDialer2);
	DDX_Control(pDX, IDC_KEY_3, m_ButtonDialer3);
	DDX_Control(pDX, IDC_KEY_4, m_ButtonDialer4);
	DDX_Control(pDX, IDC_KEY_5, m_ButtonDialer5);
	DDX_Control(pDX, IDC_KEY_6, m_ButtonDialer6);
	DDX_Control(pDX, IDC_KEY_7, m_ButtonDialer7);
	DDX_Control(pDX, IDC_KEY_8, m_ButtonDialer8);
	DDX_Control(pDX, IDC_KEY_9, m_ButtonDialer9);
	DDX_Control(pDX, IDC_KEY_0, m_ButtonDialer0);
	DDX_Control(pDX, IDC_KEY_STAR, m_ButtonDialerStar);
	DDX_Control(pDX, IDC_KEY_GRATE, m_ButtonDialerGrate);
	DDX_Control(pDX, IDC_REDIAL, m_ButtonDialerRedial);
	DDX_Control(pDX, IDC_DELETE, m_ButtonDialerDelete);
	DDX_Control(pDX, IDC_KEY_PLUS, m_ButtonDialerPlus);
	DDX_Control(pDX, IDC_CLEAR, m_ButtonDialerClear);
}

void Dialer::RebuildShortcuts(bool init)
{
	if (!mainDlg->shortcutsEnabled) {
		return;
	}
	CRect rect;
	if (!init) {
		mainDlg->GetWindowRect(rect);
		mainDlg->SetWindowPos(NULL, 0, 0, mainDlg->windowSize.x, mainDlg->windowSize.y, SWP_NOZORDER | SWP_NOMOVE);
		//--		
		POSITION pos = shortcutButtons.GetHeadPosition();
		while (pos) {
			POSITION posKey = pos;
			CButton* button = shortcutButtons.GetNext(pos);
			AutoUnmove(button->m_hWnd);
			button->DestroyWindow();
			delete button;
			shortcutButtons.RemoveAt(posKey);
		};
		//--
	}
	if (shortcuts.GetCount()) {
		CFont* font = this->GetFont();
		CRect shortcutsRect;
		GetWindowRect(shortcutsRect);
		ScreenToClient(shortcutsRect);

		CRect rectVoicemail;
		m_ButtonVoicemail.GetWindowRect(&rectVoicemail);
		ScreenToClient(rectVoicemail);
		CRect mapRect;
		int buttonHeight;
		int moveFactor;
		int moveFix;
		if (mainDlg->shortcutsBottom) {
			mapRect.top = 8;
			mapRect.left = 4;
			MapDialogRect(&mapRect);
			shortcutsRect.top = rectVoicemail.bottom + mapRect.top;
			buttonHeight = 25;
			moveFactor = 0;
		}
		else {
			mapRect.top = 2;
			mapRect.bottom = 1;
			MapDialogRect(&mapRect);
			shortcutsRect.top += mapRect.top;
			shortcutsRect.bottom -= mapRect.bottom;
			buttonHeight = shortcutsRect.Height() / shortcuts.GetCount();
			shortcutsRect.top = shortcutsRect.top + (shortcutsRect.Height() - buttonHeight * shortcuts.GetCount())/2;
			moveFactor = 100 / shortcuts.GetCount();
		}
		for (int i = 0; i < shortcuts.GetCount(); i++) {
			Shortcut shortcut = shortcuts.GetAt(i);
			CButton *button = new CButton();
			if (mainDlg->shortcutsBottom) {
				CRect buttonRect;
				buttonRect = CRect(shortcutsRect.left + mapRect.left, shortcutsRect.top, shortcutsRect.right - mapRect.left, shortcutsRect.top + buttonHeight);
				button->Create(shortcut.label, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, buttonRect, this, IDC_SHORTCUT_RANGE + i);
				AutoMove(button->m_hWnd, 0, 100, 100, 0);
				shortcutsRect.top += buttonHeight;
			}
			else {
				button->Create(shortcut.label, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, CRect(shortcutsRect.right - 138, shortcutsRect.top, shortcutsRect.right - 4, shortcutsRect.top + buttonHeight), this, IDC_SHORTCUT_RANGE + i);
				AutoMove(button->m_hWnd, 100, i * moveFactor, 0, moveFactor);
				shortcutsRect.top += buttonHeight;
			}
			button->SetFont(font);
			shortcutButtons.AddTail(button);
		}
	}
	if (!init) {
		mainDlg->SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE);
	}
}

BOOL Dialer::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	m_hCursorHand = ::LoadCursor(NULL, IDC_HAND);
	CFont* font = this->GetFont();

	RebuildShortcuts(true);

	TranslateDialog(this->m_hWnd);

	RebuildButtons(true);

	UpdateVoicemailButton(false);

	AutoMove(IDC_NUMBER, 0, 0, 100, 0);

	int height = 17;
	int height4 = height * 4;
	int height2 = height * 2;
	int height3 = height * 3;
	AutoMove(IDC_KEY_1, 0, 0, 33, height);
	AutoMove(IDC_KEY_4, 0, height, 33, height);
	AutoMove(IDC_KEY_7, 0, height2, 33, height);
	AutoMove(IDC_KEY_STAR, 0, height3, 33, height);
	AutoMove(IDC_REDIAL, 0, height4, 33, height);
	AutoMove(IDC_DELETE, 0, height4, 33, 17);

	AutoMove(IDC_KEY_2, 33, 0, 34, height);
	AutoMove(IDC_KEY_5, 33, height, 34, height);
	AutoMove(IDC_KEY_8, 33, height2, 34, height);
	AutoMove(IDC_KEY_0, 33, height3, 34, height);
	AutoMove(IDC_KEY_PLUS, 33, height4, 34, height);

	AutoMove(IDC_KEY_3, 67, 0, 33, height);
	AutoMove(IDC_KEY_6, 67, height, 33, height);
	AutoMove(IDC_KEY_9, 67, height2, 33, height);
	AutoMove(IDC_KEY_GRATE, 67, height3, 33, height);
	AutoMove(IDC_CLEAR, 67, height4, 33, height);

#ifdef _GLOBAL_VIDEO
	AutoMove(IDC_VIDEO_CALL, 0, 85, 14, 15);
	AutoMove(IDC_CALL, 14, 85, 72, 15);
	AutoMove(IDC_MESSAGE, 86, 85, 14, 15);
#else
	AutoMove(IDC_CALL, 0, 85, 84, 15);
	AutoMove(IDC_MESSAGE, 84, 85, 16, 15);
#endif

	AutoMove(IDC_HOLD, 0, 85, 14, 15);
	AutoMove(IDC_END, 14, 85, 72, 15);
	AutoMove(IDC_TRANSFER, 86, 85, 14, 15);

	AutoMove(IDC_BUTTON_MUTE_OUTPUT, 0, 100, 0, 0);
	AutoMove(IDC_BUTTON_MUTE_INPUT, 0, 100, 0, 0);
	AutoMove(IDC_VOLUME_INPUT, 0, 100, 100, 0);
	AutoMove(IDC_VOLUME_OUTPUT, 0, 100, 100, 0);

	AutoMove(IDC_DIALER_VOICEMAIL, 100, 100, 0, 0);


	DialedLoad();

	CDC *pDC = GetDC();
	int dpiY = GetDeviceCaps(pDC->m_hDC, LOGPIXELSY);
	LOGFONT lf;
	font->GetLogFont(&lf);
	//--
	m_font_call.CreateFontIndirect(&lf);
	//--
	lf.lfHeight = MulDiv(22, dpiY, 96);
	m_font.CreateFontIndirect(&lf);
	//--
	m_font_number.CreateFontIndirect(&lf);
	//--
	ReleaseDC(pDC);
	//--
	CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
	combobox->SetWindowPos(NULL, 0, 0, combobox->GetDroppedWidth(), 400, SWP_NOZORDER | SWP_NOMOVE);
	combobox->SetFont(&m_font_number);


	GetDlgItem(IDC_KEY_1)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_2)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_3)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_4)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_5)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_6)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_7)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_8)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_9)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_0)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_STAR)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_GRATE)->SetFont(&m_font);
	GetDlgItem(IDC_KEY_PLUS)->SetFont(&m_font);
	GetDlgItem(IDC_CLEAR)->SetFont(&m_font);
	GetDlgItem(IDC_REDIAL)->SetFont(&m_font);
	GetDlgItem(IDC_DELETE)->SetFont(&m_font);

	GetDlgItem(IDC_CALL)->SetFont(&m_font_call);
	GetDlgItem(IDC_END)->SetFont(&m_font_call);

	if (m_ToolTip.Create(this)) {
		m_ToolTip.AddTool(&m_ButtonDialerRedial, Translate(_T("Redial")));
		m_ToolTip.AddTool(&m_ButtonDialerDelete, Translate(_T("Backspace")));
		m_ToolTip.AddTool(&m_ButtonDialerClear, Translate(_T("Clear")));
		m_ToolTip.AddTool(&m_ButtonVoicemail, Translate(_T("Voicemail Number")));
		m_ToolTip.Activate(TRUE);
	}

	muteOutput = FALSE;
	muteInput = FALSE;

	m_SliderCtrlOutput.SetRange(0, 100);
	m_SliderCtrlOutput.SetPos(accountSettings.volumeOutput);

	m_SliderCtrlInput.SetRange(0, 100);
	m_SliderCtrlInput.SetPos(accountSettings.volumeInput);

	m_hIconMuteOutput = LoadImageIcon(IDI_MUTE_OUTPUT);
	((CButton*)GetDlgItem(IDC_BUTTON_MUTE_OUTPUT))->SetIcon(m_hIconMuteOutput);
	m_hIconMutedOutput = LoadImageIcon(IDI_MUTED_OUTPUT);

	m_hIconMuteInput = LoadImageIcon(IDI_MUTE_INPUT);
	((CButton*)GetDlgItem(IDC_BUTTON_MUTE_INPUT))->SetIcon(m_hIconMuteInput);
	m_hIconMutedInput = LoadImageIcon(IDI_MUTED_INPUT);

	m_hIconHold = LoadImageIcon(IDI_HOLD);
	((CButton*)GetDlgItem(IDC_HOLD))->SetIcon(m_hIconHold);
	m_hIconTransfer = LoadImageIcon(IDI_TRANSFER);
	((CButton*)GetDlgItem(IDC_TRANSFER))->SetIcon(m_hIconTransfer);
#ifdef _GLOBAL_VIDEO
	m_hIconVideo = LoadImageIcon(IDI_VIDEO);
	((CButton*)GetDlgItem(IDC_VIDEO_CALL))->SetIcon(m_hIconVideo);
#endif
	m_hIconMessage = LoadImageIcon(IDI_MESSAGE);
	((CButton*)GetDlgItem(IDC_MESSAGE))->SetIcon(m_hIconMessage);

	return TRUE;
}

int Dialer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (mainDlg->widthAdd || mainDlg->heightAdd) {
		SetWindowPos(NULL, 0, 0, lpCreateStruct->cx + mainDlg->widthAdd, lpCreateStruct->cy + mainDlg->heightAdd, SWP_NOMOVE | SWP_NOZORDER);
	}
	return CBaseDialog::OnCreate(lpCreateStruct);
}

void Dialer::OnDestroy()
{
	KillTimer(IDT_TIMER_VU_METER);
	CBaseDialog::OnDestroy();
}

void Dialer::PostNcDestroy()
{
	CBaseDialog::PostNcDestroy();
	delete this;
}

BEGIN_MESSAGE_MAP(Dialer, CBaseDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_DIALER_DND, &Dialer::OnBnClickedDND)
	ON_BN_CLICKED(IDC_DIALER_AA, &Dialer::OnBnClickedAA)
	ON_BN_CLICKED(IDC_DIALER_VOICEMAIL, OnBnClickedVoicemail)
	ON_BN_CLICKED(IDC_BUTTON_PLUS_INPUT, &Dialer::OnBnClickedPlusInput)
	ON_BN_CLICKED(IDC_BUTTON_MINUS_INPUT, &Dialer::OnBnClickedMinusInput)
	ON_BN_CLICKED(IDC_BUTTON_PLUS_OUTPUT, &Dialer::OnBnClickedPlusOutput)
	ON_BN_CLICKED(IDC_BUTTON_MINUS_OUTPUT, &Dialer::OnBnClickedMinusOutput)
	ON_BN_CLICKED(IDC_BUTTON_MUTE_OUTPUT, &Dialer::OnBnClickedMuteOutput)
	ON_BN_CLICKED(IDC_BUTTON_MUTE_INPUT, &Dialer::OnBnClickedMuteInput)
	ON_COMMAND_RANGE(IDC_SHORTCUT_RANGE, IDC_SHORTCUT_RANGE + 20, &Dialer::OnBnClickedShortcut)
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()

	ON_BN_CLICKED(IDC_CALL, OnBnClickedCall)
#ifdef _GLOBAL_VIDEO
	ON_BN_CLICKED(IDC_VIDEO_CALL, OnBnClickedVideoCall)
#endif
	ON_BN_CLICKED(IDC_MESSAGE, OnBnClickedMessage)
	ON_BN_CLICKED(IDC_HOLD, OnBnClickedHold)
	ON_BN_CLICKED(IDC_TRANSFER, OnBnClickedTransfer)
	ON_BN_CLICKED(IDC_END, OnBnClickedEnd)
	ON_CBN_EDITCHANGE(IDC_NUMBER, &Dialer::OnCbnEditchangeComboAddr)
	ON_CBN_SELCHANGE(IDC_NUMBER, &Dialer::OnCbnSelchangeComboAddr)

	ON_BN_CLICKED(IDC_KEY_1, &Dialer::OnBnClickedKey1)
	ON_BN_CLICKED(IDC_KEY_2, &Dialer::OnBnClickedKey2)
	ON_BN_CLICKED(IDC_KEY_3, &Dialer::OnBnClickedKey3)
	ON_BN_CLICKED(IDC_KEY_4, &Dialer::OnBnClickedKey4)
	ON_BN_CLICKED(IDC_KEY_5, &Dialer::OnBnClickedKey5)
	ON_BN_CLICKED(IDC_KEY_6, &Dialer::OnBnClickedKey6)
	ON_BN_CLICKED(IDC_KEY_7, &Dialer::OnBnClickedKey7)
	ON_BN_CLICKED(IDC_KEY_8, &Dialer::OnBnClickedKey8)
	ON_BN_CLICKED(IDC_KEY_9, &Dialer::OnBnClickedKey9)
	ON_BN_CLICKED(IDC_KEY_STAR, &Dialer::OnBnClickedKeyStar)
	ON_BN_CLICKED(IDC_KEY_0, &Dialer::OnBnClickedKey0)
	ON_BN_CLICKED(IDC_KEY_GRATE, &Dialer::OnBnClickedKeyGrate)
	ON_BN_CLICKED(IDC_REDIAL, &Dialer::OnBnClickedRedial)
	ON_BN_CLICKED(IDC_DELETE, &Dialer::OnBnClickedDelete)
	ON_BN_CLICKED(IDC_KEY_PLUS, &Dialer::OnBnClickedKeyPlus)
	ON_BN_CLICKED(IDC_CLEAR, &Dialer::OnBnClickedClear)
	ON_WM_HSCROLL()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()

void Dialer::UpdateVoicemailButton(bool hasMail)
{
	if (hasMail) {
		m_ButtonVoicemail.LoadBitmaps(IDB_VMAIL, IDB_VMAIL_DOWN, IDB_VMAIL_FOCUS);
	}
	else {
		m_ButtonVoicemail.LoadBitmaps(IDB_VMAIL_GREY, IDB_VMAIL_GREY_DOWN, IDB_VMAIL_GREY_FOCUS);
	}
	m_ButtonVoicemail.SizeToContent();

}

void Dialer::RebuildButtons(bool init)
{
	m_ButtonVoicemail.ShowWindow(SW_SHOW);
	if (IsChild(&m_ButtonDND)) {
		m_ToolTip.DelTool(&m_ButtonDND);
		m_ButtonDND.DestroyWindow();
	}
	if (IsChild(&m_ButtonAA)) {
		m_ToolTip.DelTool(&m_ButtonAA);
		m_ButtonAA.DestroyWindow();
	}
	bool addAA = accountSettings.autoAnswer == _T("button");
	bool addDND = accountSettings.denyIncoming == _T("button");
	if (addAA || addDND) {
		CRect mainRect;
		if (!init) {
			mainDlg->GetWindowRect(mainRect);
			mainDlg->SetWindowPos(NULL, 0, 0, mainDlg->windowSize.x, mainDlg->windowSize.y, SWP_NOZORDER | SWP_NOMOVE);
		}

		CRect rect;
		m_ButtonVoicemail.GetWindowRect(&rect);
		ScreenToClient(rect);
		rect.top -= 1;
		rect.bottom += 2;
		rect.left -= 1;
		rect.right += 2;

		CRect mapRect;
		mapRect.bottom = 2;
		MapDialogRect(&mapRect);
		int stepPx = mapRect.bottom + rect.Width();

		rect.left -= stepPx;
		rect.right -= stepPx;

		if (addAA) {
			m_ButtonAA.Create(Translate(_T("AA")), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX | BS_PUSHLIKE, rect, this, IDC_DIALER_AA);
			m_ButtonAA.SetFont(GetFont());
			m_ButtonAA.SetCheck(accountSettings.AA ? BST_CHECKED : BST_UNCHECKED);
			AutoMove(m_ButtonAA.m_hWnd, 100, 100, 0, 0);
			m_ToolTip.AddTool(&m_ButtonAA, Translate(_T("Auto Answer")));
			rect.left -= stepPx;
			rect.right -= stepPx;
		}
		if (addDND) {
			m_ButtonDND.Create(Translate(_T("DND")), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX | BS_PUSHLIKE, rect, this, IDC_DIALER_DND);
			m_ButtonDND.SetFont(GetFont());
			m_ButtonDND.SetCheck(accountSettings.DND ? BST_CHECKED : BST_UNCHECKED);
			AutoMove(m_ButtonDND.m_hWnd, 100, 100, 0, 0);
			m_ToolTip.AddTool(&m_ButtonDND, Translate(_T("Do Not Disturb")));
			rect.left -= stepPx;
			rect.right -= stepPx;
		}
		if (!init) {
			mainDlg->SetWindowPos(NULL, 0, 0, mainRect.Width(), mainRect.Height(), SWP_NOZORDER | SWP_NOMOVE);
		}
	}
}

void Dialer::OnTimer(UINT_PTR TimerVal)
{
	if (TimerVal == IDT_TIMER_VU_METER) {
		TimerVuMeter();
	}
}

BOOL Dialer::PreTranslateMessage(MSG* pMsg)
{
	m_ToolTip.RelayEvent(pMsg);

	BOOL catched = FALSE;
	BOOL isEdit = FALSE;
	CEdit* edit = NULL;
	if (pMsg->message == WM_CHAR || (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)) {
		CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
		edit = (CEdit*)FindWindowEx(combobox->m_hWnd, NULL, _T("EDIT"), NULL);
		isEdit = !edit || edit == GetFocus();
	}
	if (pMsg->message == WM_CHAR)
	{
		if (pMsg->wParam == 48)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_0));
				OnBnClickedKey0();
				catched = TRUE;
			}
			else {
				DTMF(_T("0"));
			}
		}
		else if (pMsg->wParam == 49)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_1));
				OnBnClickedKey1();
				catched = TRUE;
			}
			else {
				DTMF(_T("1"));
			}
		}
		else if (pMsg->wParam == 50)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_2));
				OnBnClickedKey2();
				catched = TRUE;
			}
			else {
				DTMF(_T("2"));
			}
		}
		else if (pMsg->wParam == 51)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_3));
				OnBnClickedKey3();
				catched = TRUE;
			}
			else {
				DTMF(_T("3"));
			}
		}
		else if (pMsg->wParam == 52)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_4));
				OnBnClickedKey4();
				catched = TRUE;
			}
			else {
				DTMF(_T("4"));
			}
		}
		else if (pMsg->wParam == 53)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_5));
				OnBnClickedKey5();
				catched = TRUE;
			}
			else {
				DTMF(_T("5"));
			}
		}
		else if (pMsg->wParam == 54)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_6));
				OnBnClickedKey6();
				catched = TRUE;
			}
			else {
				DTMF(_T("6"));
			}
		}
		else if (pMsg->wParam == 55)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_7));
				OnBnClickedKey7();
				catched = TRUE;
			}
			else {
				DTMF(_T("7"));
			}
		}
		else if (pMsg->wParam == 56)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_8));
				OnBnClickedKey8();
				catched = TRUE;
			}
			else {
				DTMF(_T("8"));
			}
		}
		else if (pMsg->wParam == 57)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_9));
				OnBnClickedKey9();
				catched = TRUE;
			}
			else {
				DTMF(_T("9"));
			}
		}
		else if (pMsg->wParam == 35 || pMsg->wParam == 47)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_GRATE));
				OnBnClickedKeyGrate();
				catched = TRUE;
			}
			else {
				DTMF(_T("#"));
			}
		}
		else if (pMsg->wParam == 42)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_STAR));
				OnBnClickedKeyStar();
				catched = TRUE;
			}
			else {
				DTMF(_T("*"));
			}
		}
		else if (pMsg->wParam == 43)
		{
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_KEY_PLUS));
				OnBnClickedKeyPlus();
				catched = TRUE;
			}
		}
		else if (pMsg->wParam == 8 || pMsg->wParam == 45)
		{
			if (!isEdit)
			{
				GotoDlgCtrl(GetDlgItem(IDC_DELETE));
				OnBnClickedDelete();
				catched = TRUE;
			}
		}
		else if (pMsg->wParam == 46)
		{
			if (!isEdit)
			{
				Input(_T("."), TRUE);
				catched = TRUE;
			}
		}
	}
	else if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_ESCAPE) {
			if (!isEdit) {
				GotoDlgCtrl(GetDlgItem(IDC_NUMBER));
				catched = TRUE;
			}
			if (edit) {
				CString str;
				edit->GetWindowText(str);
				if (!str.IsEmpty()) {
					Clear();
					catched = TRUE;
				}
			}
		}
	}
	if (!catched)
	{
		return CBaseDialog::PreTranslateMessage(pMsg);
	}
	else {
		return TRUE;
	}
}

void Dialer::OnBnClickedOk()
{
	if (accountSettings.singleMode && GetDlgItem(IDC_END)->IsWindowVisible()) {
		OnBnClickedEnd();
	}
	else {
		OnBnClickedCall();
	}
}

void Dialer::OnBnClickedCancel()
{
	mainDlg->ShowWindow(SW_HIDE);
}

void Dialer::DTMF(CString digits)
{
	pjsua_call_id call_id = PJSUA_INVALID_ID;
	MessagesContact*  messagesContact = mainDlg->messagesDlg->GetMessageContact();
	if (messagesContact && messagesContact->callId != -1) {
		call_id = messagesContact->callId;
	}
	msip_call_dial_dtmf(call_id, digits);
}

void Dialer::Input(CString digits, BOOL disableDTMF)
{
	if (!disableDTMF) {
		DTMF(digits);
	}
	CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
	CEdit* edit = (CEdit*)FindWindowEx(combobox->m_hWnd, NULL, _T("EDIT"), NULL);
	if (edit) {
		int nLength = edit->GetWindowTextLength();
		edit->SetSel(nLength, nLength);
		edit->ReplaceSel(digits);
	}
}

void Dialer::DialedClear()
{
	CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
	combobox->ResetContent();
	combobox->Clear();
}
void Dialer::DialedLoad()
{
	CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
	CString key;
	CString val;
	LPTSTR ptr = val.GetBuffer(255);
	int i = 0;
	while (TRUE) {
		key.Format(_T("%d"), i);
		if (GetPrivateProfileString(_T("Dialed"), key, NULL, ptr, 256, accountSettings.iniFile)) {
			combobox->AddString(ptr);
		}
		else {
			break;
		}
		i++;
	}
}

void Dialer::DialedSave(CComboBox *combobox)
{
	CString key;
	CString val;
	WritePrivateProfileString(_T("Dialed"), NULL, NULL, accountSettings.iniFile);
	for (int i = 0; i < combobox->GetCount(); i++)
	{
		int n = combobox->GetLBTextLen(i);
		combobox->GetLBText(i, val.GetBuffer(n));
		val.ReleaseBuffer();

		key.Format(_T("%d"), i);
		WritePrivateProfileString(_T("Dialed"), key, val, accountSettings.iniFile);
	}
}

void Dialer::SetNumber(CString  number, int callsCount)
{
	CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
	CString old;
	combobox->GetWindowText(old);
	if (old.IsEmpty() || number.Find(old) != 0) {
		combobox->SetWindowText(number);
	}
	UpdateCallButton(0, callsCount);
}

void Dialer::UpdateCallButton(BOOL forse, int callsCount)
{
	int len;
	if (!forse) {
		CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
		len = combobox->GetWindowTextLength();
	}
	else {
		len = 1;
	}
	CButton *button = (CButton *)GetDlgItem(IDC_CALL);
	bool state = false;
	if (accountSettings.singleMode) {
		if (callsCount == -1) {
			callsCount = call_get_count_noincoming();
		}
		if (callsCount) {
			if (!GetDlgItem(IDC_END)->IsWindowVisible()) {
				button->ShowWindow(SW_HIDE);
#ifdef _GLOBAL_VIDEO
				GetDlgItem(IDC_VIDEO_CALL)->ShowWindow(SW_HIDE);
#endif
				GetDlgItem(IDC_MESSAGE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_HOLD)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_TRANSFER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_END)->ShowWindow(SW_SHOW);
				GotoDlgCtrl(GetDlgItem(IDC_END));
			}
		}
		else {
			if (GetDlgItem(IDC_END)->IsWindowVisible()) {
				GetDlgItem(IDC_HOLD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_TRANSFER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_END)->ShowWindow(SW_HIDE);
				button->ShowWindow(SW_SHOW);
#ifdef _GLOBAL_VIDEO
				GetDlgItem(IDC_VIDEO_CALL)->ShowWindow(SW_SHOW);
#endif
				GetDlgItem(IDC_MESSAGE)->ShowWindow(SW_SHOW);
			}
		}
		state = callsCount || len ? true : false;

	}
	else {
		state = len ? true : false;
	}
	button->EnableWindow(state);

#ifdef _GLOBAL_VIDEO
	GetDlgItem(IDC_VIDEO_CALL)->EnableWindow(state);
#endif
	GetDlgItem(IDC_MESSAGE)->EnableWindow(state);

	CButton *buttonRedial = (CButton *)GetDlgItem(IDC_REDIAL);
	CButton *buttonDelete = (CButton *)GetDlgItem(IDC_DELETE);
	if (!state) {
		buttonDelete->ShowWindow(SW_HIDE);
		buttonRedial->ShowWindow(SW_SHOW);
	}
	else {
		buttonRedial->ShowWindow(SW_HIDE);
		buttonDelete->ShowWindow(SW_SHOW);
	}
}

void Dialer::Action(DialerActions action)
{
	CString number;
	CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
	combobox->GetWindowText(number);
	number.Trim();
	if (!number.IsEmpty()) {
		bool res = false;
		if (action != ACTION_MESSAGE) {
			res = mainDlg->MakeCall(number, action == ACTION_VIDEO_CALL);
		}
		else {
			res = mainDlg->MessagesOpen(number);
		}
		if (res) {
			//-- save dialed in combobox
			int pos = combobox->FindStringExact(-1, number);
			if (pos == CB_ERR || pos > 0) {
				if (pos > 0) {
					combobox->DeleteString(pos);
				}
				else if (combobox->GetCount() >= 10)
				{
					combobox->DeleteString(combobox->GetCount() - 1);
				}
				combobox->InsertString(0, number);
				combobox->SetCurSel(0);
			}
			DialedSave(combobox);
			if (!accountSettings.singleMode) {
				Clear();
			}
			//-- end
		}
	}
}

void Dialer::Clear(bool update)
{
	CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
	combobox->SetCurSel(-1);
	if (update) {
		UpdateCallButton();
	}
}

void Dialer::OnBnClickedCall()
{
	Action(ACTION_CALL);
}

#ifdef _GLOBAL_VIDEO
void Dialer::OnBnClickedVideoCall()
{
	Action(ACTION_VIDEO_CALL);
}
#endif

void Dialer::OnBnClickedMessage()
{
	Action(ACTION_MESSAGE);
}

void Dialer::OnBnClickedHold()
{
	mainDlg->messagesDlg->OnBnClickedHold();
}

void Dialer::OnBnClickedTransfer()
{
	if (!mainDlg->transferDlg) {
		mainDlg->transferDlg = new Transfer(this);
	}
	mainDlg->transferDlg->SetAction(MSIP_ACTION_TRANSFER);
	mainDlg->transferDlg->SetForegroundWindow();
}

void Dialer::OnBnClickedEnd()
{
	MessagesContact*  messagesContact = mainDlg->messagesDlg->GetMessageContact();
	if (messagesContact && messagesContact->callId != -1) {
		msip_call_end(messagesContact->callId);
	}
	else {
		call_hangup_all_noincoming();
	}
}

void Dialer::OnCbnEditchangeComboAddr()
{
	UpdateCallButton();
}

void Dialer::OnCbnSelchangeComboAddr()
{
	UpdateCallButton(TRUE);
}

void Dialer::OnBnClickedKey1()
{
	Input(_T("1"));
}

void Dialer::OnBnClickedKey2()
{
	Input(_T("2"));
}

void Dialer::OnBnClickedKey3()
{
	Input(_T("3"));
}

void Dialer::OnBnClickedKey4()
{
	Input(_T("4"));
}

void Dialer::OnBnClickedKey5()
{
	Input(_T("5"));
}

void Dialer::OnBnClickedKey6()
{
	Input(_T("6"));
}

void Dialer::OnBnClickedKey7()
{
	Input(_T("7"));
}

void Dialer::OnBnClickedKey8()
{
	Input(_T("8"));
}

void Dialer::OnBnClickedKey9()
{
	Input(_T("9"));
}

void Dialer::OnBnClickedKeyStar()
{
	Input(_T("*"));
}

void Dialer::OnBnClickedKey0()
{
	Input(_T("0"));
}

void Dialer::OnBnClickedKeyGrate()
{
	Input(_T("#"));
}

void Dialer::OnBnClickedRedial()
{
	if (!accountSettings.lastCallNumber.IsEmpty()) {
		mainDlg->MakeCall(accountSettings.lastCallNumber, accountSettings.lastCallHasVideo);
	}
}

void Dialer::OnBnClickedDelete()
{
	CComboBox *combobox = (CComboBox*)GetDlgItem(IDC_NUMBER);
	CEdit* edit = (CEdit*)FindWindowEx(combobox->m_hWnd, NULL, _T("EDIT"), NULL);
	if (edit) {
		int nLength = edit->GetWindowTextLength();
		edit->SetSel(nLength - 1, nLength);
		edit->ReplaceSel(_T(""));
	}
}

void Dialer::OnBnClickedKeyPlus()
{
	Input(_T("+"), TRUE);
}

void Dialer::OnBnClickedClear()
{
	Clear();
}

void Dialer::OnLButtonUp(UINT nFlags, CPoint pt)
{
}

void Dialer::OnRButtonUp(UINT nFlags, CPoint pt)
{
}

void Dialer::OnMouseMove(UINT nFlags, CPoint pt)
{
}

void Dialer::OnHScroll(UINT, UINT, CScrollBar* sender)
{
	if (pj_ready) {
		int pos;
		if (!sender || sender == (CScrollBar*)&m_SliderCtrlOutput) {
			if (sender && muteOutput) {
				CButton *button = (CButton*)GetDlgItem(IDC_BUTTON_MUTE_OUTPUT);
				button->SetCheck(BST_UNCHECKED);
				OnBnClickedMuteOutput();
				return;
			}
			pos = m_SliderCtrlOutput.GetPos();
			//msip_audio_output_set_volume(pos,muteOutput);
			msip_audio_conf_set_volume(pos, muteOutput);
			accountSettings.volumeOutput = pos;
			mainDlg->AccountSettingsPendingSave();
		}
		if (!sender || sender == (CScrollBar*)&m_SliderCtrlInput) {
			if (sender && muteInput) {
				CButton *button = (CButton*)GetDlgItem(IDC_BUTTON_MUTE_INPUT);
				button->SetCheck(BST_UNCHECKED);
				OnBnClickedMuteInput();
				return;
			}
			pos = m_SliderCtrlInput.GetPos();
			msip_audio_input_set_volume(pos, muteInput);
			accountSettings.volumeInput = pos;
			mainDlg->AccountSettingsPendingSave();
		}
	}
}

void Dialer::OnBnClickedMinusInput()
{
	int pos = m_SliderCtrlInput.GetPos();
	if (pos > 0) {
		pos -= 5;
		if (pos < 0) {
			pos = 0;
		}
		m_SliderCtrlInput.SetPos(pos);
		OnHScroll(0, 0, (CScrollBar *)&m_SliderCtrlInput);
	}


}

void Dialer::OnBnClickedPlusInput()
{
	int pos = m_SliderCtrlInput.GetPos();
	if (pos < 100) {
		pos += 5;
		if (pos > 100) {
			pos = 100;
		}
		m_SliderCtrlInput.SetPos(pos);
		OnHScroll(0, 0, (CScrollBar *)&m_SliderCtrlInput);
	}
}

void Dialer::OnBnClickedMinusOutput()
{
	int pos = m_SliderCtrlOutput.GetPos();
	if (pos > 0) {
		pos -= 5;
		if (pos < 0) {
			pos = 0;
		}
		m_SliderCtrlOutput.SetPos(pos);
		OnHScroll(0, 0, (CScrollBar *)&m_SliderCtrlOutput);
	}
}

void Dialer::OnBnClickedPlusOutput()
{
	int pos = m_SliderCtrlOutput.GetPos();
	if (pos < 100) {
		pos += 5;
		if (pos > 100) {
			pos = 100;
		}
		m_SliderCtrlOutput.SetPos(pos);
		OnHScroll(0, 0, (CScrollBar *)&m_SliderCtrlOutput);
	}
}

void Dialer::OnBnClickedMuteOutput()
{
	CButton *button = (CButton*)GetDlgItem(IDC_BUTTON_MUTE_OUTPUT);
	if (button->GetCheck() == BST_CHECKED) {
		button->SetIcon(m_hIconMuteOutput);
		muteOutput = FALSE;
		OnHScroll(0, 0, NULL);
	}
	else {
		button->SetIcon(m_hIconMutedOutput);
		muteOutput = TRUE;
		OnHScroll(0, 0, NULL);
	}
	button->SetCheck(!button->GetCheck());
}

void Dialer::OnBnClickedMuteInput()
{
	CButton *button = (CButton*)GetDlgItem(IDC_BUTTON_MUTE_INPUT);
	if (button->GetCheck() == BST_CHECKED) {
		button->SetIcon(m_hIconMuteInput);
		muteInput = FALSE;
		OnHScroll(0, 0, NULL);
	}
	else {
		button->SetIcon(m_hIconMutedInput);
		muteInput = TRUE;
		OnHScroll(0, 0, NULL);
	}
	button->SetCheck(!button->GetCheck());
}

void Dialer::TimerVuMeter()
{
	unsigned tx_level = 0, rx_level = 0;
	pjsua_conf_port_id ids[PJSUA_MAX_CONF_PORTS];
	unsigned count = PJSUA_MAX_CONF_PORTS;
	if (pjsua_var.state == PJSUA_STATE_RUNNING && pjsua_enum_conf_ports(ids, &count) == PJ_SUCCESS && count > 1) {
		for (unsigned i = 0; i < count; i++) {
			unsigned tx_level_curr, rx_level_curr;
			pjsua_conf_port_info conf_port_info;
			if (pjsua_conf_get_port_info(ids[i], &conf_port_info) == PJ_SUCCESS) {
				if (pjsua_conf_get_signal_level(ids[i], &tx_level_curr, &rx_level_curr) == PJ_SUCCESS) {
					if (conf_port_info.slot_id == 0) {
						tx_level = rx_level_curr * (conf_port_info.rx_level_adj > 0 ? 1 : 0);
					}
					else {
						rx_level_curr = conf_port_info.rx_level_adj > 0 ? rx_level_curr : 0;
						if (rx_level_curr > rx_level) {
							rx_level = rx_level_curr;
						}
					}
				}
			}
		}
		if (!m_SliderCtrlInput.IsActive) m_SliderCtrlInput.IsActive = true;
		if (!m_SliderCtrlOutput.IsActive) m_SliderCtrlOutput.IsActive = true;
	}
	else {
		KillTimer(IDT_TIMER_VU_METER);
		m_SliderCtrlInput.IsActive = false;
		m_SliderCtrlOutput.IsActive = false;
	}
	//CString s;
	//s.Format(_T("tx %d rx %d"),tx_level_max, tx_level_max);
	//mainDlg->SetWindowText(s);
	m_SliderCtrlInput.SetSelection(0, tx_level / 0.95);
	m_SliderCtrlInput.Invalidate(FALSE);
	m_SliderCtrlOutput.SetSelection(0, rx_level / 1.15);
	m_SliderCtrlOutput.Invalidate(FALSE);
}

BOOL Dialer::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (pWnd == &m_ButtonVoicemail) {
		::SetCursor(m_hCursorHand);
		return TRUE;
	}
	return CBaseDialog::OnSetCursor(pWnd, nHitTest, message);
}

void Dialer::OnBnClickedDND()
{
	CButton *button = (CButton*)GetDlgItem(IDC_DIALER_DND);
	accountSettings.DND = button->GetCheck() == BST_CHECKED;
	mainDlg->PublishStatus();
	accountSettings.SettingsSave();
}

void Dialer::OnBnClickedAA()
{
	CButton *button = (CButton*)GetDlgItem(IDC_DIALER_AA);
	accountSettings.AA = button->GetCheck() == BST_CHECKED;
	accountSettings.SettingsSave();
	mainDlg->UpdateWindowText();
}

void Dialer::OnBnClickedVoicemail()
{
	if (accountSettings.accountId && !accountSettings.account.voicemailNumber.IsEmpty()) {
		mainDlg->MakeCall(accountSettings.account.voicemailNumber);
	}
}

void Dialer::OnBnClickedShortcut(UINT nID)
{
	int i = nID - IDC_SHORTCUT_RANGE;
	mainDlg->ShortcutAction(&shortcuts.GetAt(i));
}
