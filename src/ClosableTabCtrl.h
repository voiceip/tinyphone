#pragma once

class CClosableTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CClosableTabCtrl)

public:
	CClosableTabCtrl();
	virtual ~CClosableTabCtrl();
	BOOL DeleteItem(int nItem);

	bool m_bCloseable;

protected:
	CImageList m_ImgLstCloseButton;
	IMAGEINFO m_iiCloseButton;
	CPoint m_ptCtxMenu;
	int iTabClose;

	void InternalInit();
	void SetAllIcons();
	void GetCloseButtonRect(int iItem, const CRect& rcItem, CRect& rcCloseButton, bool bItemSelected, bool bVistaThemeActive);
	int GetTabUnderContextMenu() const;
	int GetTabUnderPoint(CPoint point) const;
	bool SetDefaultContextMenuPos();
	bool IsVistaThemeActive() const;

	virtual void PreSubclassWindow();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSysColorChange();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg LRESULT _OnThemeChanged();
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMeasureItem(int, LPMEASUREITEMSTRUCT);
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT);
};
