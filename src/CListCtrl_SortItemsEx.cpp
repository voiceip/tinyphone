#include "stdafx.h"

#include "CListCtrl_SortItemsEx.h"
#include "Calls.h"
#include "mainDlg.h"

BEGIN_MESSAGE_MAP(CListCtrl_SortItemsEx, CListCtrl)
	ON_NOTIFY_REFLECT_EX(LVN_COLUMNCLICK, OnHeaderClick)	// Column Click
END_MESSAGE_MAP()

namespace {
	struct PARAMSORT
	{
		PARAMSORT(HWND hWnd, int columnIndex, bool ascending)
			:m_hWnd(hWnd)
			,m_ColumnIndex(columnIndex)
			,m_Ascending(ascending)
		{}

		HWND m_hWnd;
		int  m_ColumnIndex;
		bool m_Ascending;
	};

	// Comparison extracts values from the List-Control
	int CALLBACK SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		PARAMSORT& ps = *(PARAMSORT*)lParamSort;
		if (ps.m_ColumnIndex == 1 && mainDlg && mainDlg->pageCalls) {
			CListCtrl *list = (CListCtrl*)mainDlg->pageCalls->GetDlgItem(IDC_CALLS);
			if (list && ps.m_hWnd==list->m_hWnd) {
				LVITEM item;
				item.iItem = lParam1;
				item.iSubItem = 0;
				item.mask = LVIF_PARAM;
				ListView_GetItem(ps.m_hWnd,&item);
				Call *pCall1 = (Call *)item.lParam;
				item.iItem = lParam2;
				ListView_GetItem(ps.m_hWnd,&item);
				Call *pCall2 = (Call *)item.lParam;
				if (ps.m_Ascending) {
					if (pCall1->time > pCall2->time) return 1;
					else if (pCall1->time < pCall2->time) return -1;
					else return 0;
				} else {
					if (pCall1->time > pCall2->time) return -1;
					else if (pCall1->time < pCall2->time) return 1;
					else return 0;
				}
			}
		}
		TCHAR left[256] = _T(""), right[256] = _T("");
		ListView_GetItemText(ps.m_hWnd, lParam1, ps.m_ColumnIndex, left, sizeof(left));
		ListView_GetItemText(ps.m_hWnd, lParam2, ps.m_ColumnIndex, right, sizeof(right));	
		if (ps.m_Ascending)
			return _tcscmp( left, right );
		else
			return _tcscmp( right, left );
	}
}

bool CListCtrl_SortItemsEx::SortColumn(int columnIndex, bool ascending)
{
	PARAMSORT paramsort(m_hWnd, columnIndex, ascending);
	ListView_SortItemsEx(m_hWnd, SortFunc, &paramsort);
	return true;
}
