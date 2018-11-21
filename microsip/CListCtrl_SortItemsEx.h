#pragma once

#include "CListCtrl_Sortable.h"

class CListCtrl_SortItemsEx : public CListCtrl_Sortable
{
	DECLARE_MESSAGE_MAP();

public:
	virtual bool SortColumn(int columnIndex, bool ascending);
};