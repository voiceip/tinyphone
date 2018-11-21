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

#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>
#include "BaseDialog.h"

class RinginDlg: public CBaseDialog
{
public:
	RinginDlg(CWnd* pParent = NULL);	// standard constructor
	~RinginDlg();
	enum { IDD = IDD_RINGIN };
	pjsua_call_id call_id;
	CFont m_font;
	bool remoteHasVideo;
	CFont m_font_ignore;
	void CallAccept(BOOL hasVideo = FALSE);
private:
	void Close(BOOL accept = FALSE);
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnTimer (UINT_PTR TimerVal);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedAudio();
	afx_msg void OnBnClickedVideo();
	afx_msg void OnBnClickedDecline();
	afx_msg void OnMove(int x, int y);
};
