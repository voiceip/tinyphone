#pragma once

#include "stdafx.h"

class CLevelsSliderCtrl : public CSliderCtrl
{
public:
	CLevelsSliderCtrl(){IsActive = false;};
	bool IsActive;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *result);
};
