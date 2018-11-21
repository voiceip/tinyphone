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

#include <vector>

class CBaseDialog : public CDialog
{
	// Construction
public:
	CBaseDialog(UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor

	void AutoMove(int iID, double dXMovePct, double dYMovePct, double dXSizePct, double dYSizePct);
	void AutoMove(HWND hWnd, double dXMovePct, double dYMovePct, double dXSizePct, double dYSizePct);
	void AutoUnmove(HWND hWnd);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBaseDialog)
protected:
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CBaseDialog)
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	struct SMovingChild
	{
		HWND        m_hWnd;
		double      m_dXMoveFrac;
		double      m_dYMoveFrac;
		double      m_dXSizeFrac;
		double      m_dYSizeFrac;
		CRect       m_rcInitial;
	};
	typedef std::vector<SMovingChild>   MovingChildren;

	MovingChildren  m_MovingChildren;
	CSize           m_szInitial;
	CSize           m_szMinimum;
};
