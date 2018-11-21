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

class SettingsDlg :
	public CDialog
{
public:
	SettingsDlg(CWnd* pParent = NULL);	// standard constructor
	~SettingsDlg();
	enum { IDD = IDD_SETTINGS };

protected:
	CStringArray autoAnswerValues;
	CStringArray denyIncomingValues;

	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg LRESULT OnUpdateSettings(WPARAM wParam,LPARAM lParam);
	afx_msg void OnDeltaposSpinModify(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg int OnVKeyToItem(UINT, CListBox*, UINT);
	afx_msg void OnDeltaposSpinOrder(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkSingleMode(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkRingingSound(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkMicAmplif(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkSwAdjust(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkVAD(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkEC(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkForceCodec(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkAudioCodecs(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkVideo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkPorts(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkDnsSrv(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkStunServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkDTMFMethod(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkAutoAnswer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkDenyIncoming(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkDirectory(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkMediaButtons(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkLocalDTMF(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkEnableLog(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkBringToFrontOnIncoming(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkRandomAnswerBox(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkEnableLocal(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkCrashReport(NMHDR *pNMHDR, LRESULT *pResult);
#ifdef _GLOBAL_VIDEO
	afx_msg void OnBnClickedPreview();
#endif
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnEnChangeRingingSound();
	afx_msg void OnBnClickedDefault();
};

