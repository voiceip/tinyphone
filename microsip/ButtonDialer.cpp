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

 // ButtonDialer.cpp : implementation file
 //

#include "stdafx.h"
#include "ButtonDialer.h"
#include "Strsafe.h"
#include "const.h"

/////////////////////////////////////////////////////////////////////////////
// CButtonDialer

CButtonDialer::CButtonDialer()
{
	forceNumeric = false;
	m_map.SetAt(_T("1"), _T(""));
	m_map.SetAt(_T("2"), _T("ABC"));
	m_map.SetAt(_T("3"), _T("DEF"));
	m_map.SetAt(_T("4"), _T("GHI"));
	m_map.SetAt(_T("5"), _T("JKL"));
	m_map.SetAt(_T("6"), _T("MNO"));
	m_map.SetAt(_T("7"), _T("PQRS"));
	m_map.SetAt(_T("8"), _T("TUV"));
	m_map.SetAt(_T("9"), _T("WXYZ"));
	m_map.SetAt(_T("0"), _T(""));
	m_map.SetAt(_T("*"), _T(""));
	m_map.SetAt(_T("#"), _T(""));
}

CButtonDialer::~CButtonDialer()
{
	CloseTheme();
}


BEGIN_MESSAGE_MAP(CButtonDialer, CButton)
	ON_WM_THEMECHANGED()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CButtonDialer message handlers

void CButtonDialer::PreSubclassWindow()
{
	OpenTheme();

	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT lf;
	GetObject(hFont, sizeof(LOGFONT), &lf);
	lf.lfHeight = 12;
	CDC *pDC = GetDC();
	if (pDC) {
		dpiX = GetDeviceCaps(pDC->m_hDC, LOGPIXELSX);
		dpiY = GetDeviceCaps(pDC->m_hDC, LOGPIXELSY);
		lf.lfHeight = MulDiv(lf.lfHeight, dpiY, 96);
		ReleaseDC(pDC);
	}
	else {
		dpiX = dpiY = 96;
	}
	StringCchCopy(lf.lfFaceName, LF_FACESIZE, _T("Microsoft Sans Serif"));
	m_FontLetters.CreateFontIndirect(&lf);

	DWORD dwStyle = ::GetClassLong(m_hWnd, GCL_STYLE);
	dwStyle &= ~CS_DBLCLKS;
	::SetClassLong(m_hWnd, GCL_STYLE, dwStyle);
}

LRESULT CButtonDialer::OnThemeChanged()
{
	CloseTheme();
	OpenTheme();
	return 0L;
}

void CButtonDialer::OnSize(UINT type, int w, int h)
{
	CButton::OnSize(type, w, h);
}

void CButtonDialer::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(&rect);
	if (rect.PtInRect(point)) {
		if (GetCapture() != this) {
			SetCapture();
			Invalidate();
		}
	}
	else {
		ReleaseCapture();
		Invalidate();
	}
	CButton::OnMouseMove(nFlags, point);
}

void CButtonDialer::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);		//Get device context object
	CRect rt;
	rt = lpDrawItemStruct->rcItem;		//Get button rect
	dc.FillSolidRect(rt, dc.GetBkColor());
	dc.SetBkMode(TRANSPARENT);

	CRect rtl = rt;
	UINT state = lpDrawItemStruct->itemState;	//Get state of the button

	if (!m_hTheme) {
		UINT uStyle = DFCS_BUTTONPUSH;
		if ((state & ODS_SELECTED)) {
			uStyle |= DFCS_PUSHED;
			rtl.left += 1;
			rtl.top += 1;
		}
		dc.DrawFrameControl(rt, DFC_BUTTON, uStyle);
	}
	else {
		UINT uStyleTheme = RBS_NORMAL;
		if ((state & ODS_SELECTED)) {
			uStyleTheme = PBS_PRESSED;
		}
		else if (GetCapture() == this) {
			uStyleTheme = PBS_HOT;
		}
		DrawThemeBackground(m_hTheme, dc.m_hDC,
			BP_PUSHBUTTON, uStyleTheme,
			rt, NULL);
	}

	CString strTemp;
	GetWindowText(strTemp);		// Get the caption which have been set

	int x12 = MulDiv(12, dpiX, 96);
	int x14 = MulDiv(14, dpiX, 96);
	int x4 = MulDiv(4, dpiX, 96);

	CString letters;
	COLORREF crOldColor;
	if (!forceNumeric && m_map.Lookup(strTemp, letters)) {
		rtl.left += x14;
		dc.DrawText(strTemp, rtl, DT_LEFT | DT_VCENTER | DT_SINGLELINE);		// Draw out the caption
		HFONT hOldFont = (HFONT)SelectObject(dc.m_hDC, m_FontLetters);
		// Do your text drawing
		rtl.left += x12;
		rtl.right -= x4;
		crOldColor = dc.SetTextColor(RGB(127, 127, 127));
		dc.DrawText(letters, rtl, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		dc.SetTextColor(crOldColor);
		// Always select the old font back into the DC
		SelectObject(dc.m_hDC, hOldFont);
	}
	else {
		if (forceNumeric) {
			crOldColor = dc.SetTextColor(RGB(80, 80, 80));
		}
		else {
			crOldColor = dc.SetTextColor(RGB(127, 127, 127));
		}
		dc.DrawText(strTemp, rt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);		// Draw out the caption
		dc.SetTextColor(crOldColor);
	}

	if ((state & ODS_FOCUS))       // If the button is focused
	{
		int iChange = 3;
		rt.top += iChange;
		rt.left += iChange;
		rt.right -= iChange;
		rt.bottom -= iChange;
		dc.DrawFocusRect(rt);
	}
	dc.Detach();
}
