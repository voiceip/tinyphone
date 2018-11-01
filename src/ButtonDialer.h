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

#if !defined(AFX_DIALERBUTTON_H__8E7A4504_82E5_4C81_8A30_642AC65A4550__INCLUDED_)
#define AFX_DIALERBUTTON_H__8E7A4504_82E5_4C81_8A30_642AC65A4550__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ButtonDialer.h : header file
//

#pragma comment(lib, "UxTheme.lib")

/////////////////////////////////////////////////////////////////////////////
// CButtonDialer window

class CButtonDialer : public CButton
{
// Construction
public:
	CButtonDialer();

// Attributes
public:

// Operations
public:
	bool forceNumeric;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CButtonDialer)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CButtonDialer();
// Generated message map functions
protected:
	//{{AFX_MSG(CButtonDialer)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	
	CFont m_FontLetters;
	CMapStringToString m_map;
	HTHEME m_hTheme; 
	int dpiX;
	int dpiY;


	void OpenTheme() { m_hTheme = OpenThemeData(m_hWnd, L"Button"); }
	void CloseTheme() {
		if (m_hTheme) { CloseThemeData(m_hTheme); m_hTheme = NULL; }
	}
	DECLARE_MESSAGE_MAP()

	virtual void PreSubclassWindow();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnMouseMove(UINT,CPoint);
	afx_msg void OnSize(UINT type, int w, int h);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALERBUTTON_H__8E7A4504_82E5_4C81_8A30_642AC65A4550__INCLUDED_)
