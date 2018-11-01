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
#include "Preview.h"

#ifdef _GLOBAL_VIDEO

#include "langpack.h"
#include "mainDlg.h"
#include "settings.h"

Preview::~Preview(void)
{
}

int Preview::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (langPack.rtl) {
		ModifyStyleEx(0,WS_EX_LAYOUTRTL);
	}
	return 0;
}

Preview::Preview(CWnd* pParent)
{
	CString strClass = AfxRegisterWndClass(
		CS_HREDRAW|CS_VREDRAW,
		0,
		(HBRUSH)COLOR_GRAYTEXT
		);
	CreateEx(WS_EX_TOPMOST, strClass, Translate(_T("Local Video")), 
		WS_CAPTION | WS_POPUPWINDOW | WS_MINIMIZEBOX,
		CRect(0, 0, 640, 480),
		AfxGetMainWnd(), NULL);
	widx = PJMEDIA_VID_DEFAULT_CAPTURE_DEV;
}

BEGIN_MESSAGE_MAP(Preview, CWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void Preview::OnClose() 
{
	DestroyWindow();
}

void Preview::OnDestroy()
{
	mainDlg->previewWin = NULL;
	if (pj_ready) {
		pjsua_vid_preview_stop(widx);
	}
	CWnd::OnDestroy();
}

void Preview::PostNcDestroy()
{
	CWnd::PostNcDestroy();
	delete this;
}

void Preview::Start(int id)
{
	pjsua_vid_win_info wi;
	pjsua_vid_win_id wid = pjsua_vid_preview_get_win(widx);
	if (wid != PJSUA_INVALID_ID) {
		if (widx != id) {
			pjsua_vid_preview_stop(widx);
		} else {
			return;
		}
	}
	widx = id;

	pjsua_vid_preview_param pre_param;
	pj_status_t status;
	const pjmedia_coord pos = {0, 0};

	pjsua_vid_preview_param_default(&pre_param);
	pre_param.show = PJ_FALSE;
	status = pjsua_vid_preview_start(widx, &pre_param);
	if (status != PJ_SUCCESS) {
		OnClose();
		return;
	}
	wid = pjsua_vid_preview_get_win(widx);
	pjsua_vid_win_get_info(wid, &wi);

	CRect rcClient, rcWind;
	POINT ptDiff;
	GetClientRect(&rcClient);
	GetWindowRect(&rcWind);
	ptDiff.x =  (rcWind.right - rcWind.left) - rcClient.right;
	ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;
	CRect screenRect;
	SystemParametersInfo(SPI_GETWORKAREA,0,&screenRect,0);
	//int w = wi.size.w + ptDiff.x;
	//int h = wi.size.h + ptDiff.y;
	int w = 320 + ptDiff.x;
	int h = 240 + ptDiff.y;
	int x = screenRect.Width() - w;
	int y = ptDiff.y - 6;
	SetWindowPos(NULL, x, y, w, h, SWP_NOZORDER|SWP_SHOWWINDOW);

	pjsua_vid_win_set_pos(wid,&pos);

	pjsua_vid_win_set_show(wid, PJ_TRUE);

	const pjmedia_rect_size size = {320, 240};
	pjsua_vid_win_set_size( wid, &size);
	
	::SetParent((HWND)wi.hwnd.info.win.hwnd,this->m_hWnd);
}
#endif