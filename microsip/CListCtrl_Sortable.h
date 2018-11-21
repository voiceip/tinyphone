#pragma once

#include "CListCtrl_DataModel.h"

class CListCtrl_Sortable : public CListCtrl
{
	bool m_Ascending;
	int  m_SortCol;

	DECLARE_MESSAGE_MAP();

	afx_msg BOOL OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult);
	void PreSubclassWindow();

public:
	CListCtrl_Sortable()
		:m_Ascending(false)
		,m_SortCol(-1)
	{}
	int GetColumnData(int col) const;
	void SetSortArrow(int col, bool ascending);
	int GetSortColumn() const { return m_SortCol; }
	bool IsAscending() const { return m_Ascending; }
	void ResetSortOrder();
	virtual bool SortColumn(int columnIndex, bool ascending) = 0;
	void SetSortColumn(int columnIndex, bool ascending);
};