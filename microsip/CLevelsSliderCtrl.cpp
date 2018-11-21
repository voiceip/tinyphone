#include "CLevelsSliderCtrl.h"

BEGIN_MESSAGE_MAP(CLevelsSliderCtrl, CSliderCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()

void CLevelsSliderCtrl::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *result)
{
	NMCUSTOMDRAW *pNmcd = (NMCUSTOMDRAW *)pNotifyStruct;

	switch (pNmcd->dwDrawStage) {
	case CDDS_PREPAINT:
		if (GetStyle() & TBS_ENABLESELRANGE) {
			*result = CDRF_NOTIFYITEMDRAW;
		}
		break;
	case CDDS_ITEMPREPAINT:
		switch (pNmcd->dwItemSpec) {
		case TBCD_THUMB: {
			CDC *pDC = CDC::FromHandle(pNmcd->hdc);
			CRect rect = pNmcd->rc;
			int dpiX = GetDeviceCaps(pDC->m_hDC, LOGPIXELSX);
			int dpiY = GetDeviceCaps(pDC->m_hDC, LOGPIXELSY);
			if (GetStyle()&TBS_VERT) {
				rect.DeflateRect(MulDiv(6, dpiX, 96), MulDiv(3, dpiX, 96));
			}
			else {
				rect.DeflateRect(MulDiv(3, dpiX, 96), MulDiv(6, dpiX, 96));
			}
			pDC->FillSolidRect(&rect, RGB(170, 170, 170));
			*result = CDRF_SKIPDEFAULT;
			break;
		}
		case TBCD_CHANNEL: {
			CDC *pDC = CDC::FromHandle(pNmcd->hdc);
			int min, max, selmin, selmax;
			bool hot;
			GetRange(min, max);
			GetSelection(selmin, selmax);
			max -= min;
			CRect rect;
			GetChannelRect(&rect);
			if (GetStyle()&TBS_VERT) {
				hot = selmin <= 0;
				// fix windows bug
				if (rect.right > rect.bottom) {
					rect.left ^= rect.top ^= rect.left ^= rect.top; // swap left and top values
					rect.right ^= rect.bottom ^= rect.right ^= rect.bottom; // swap right and bottom values
				}
				rect.DeflateRect(2, 2);
				selmin = (int)(((double)(selmin - min) / max * rect.Height()) + 0.5) + rect.top;
				selmax = (int)(((double)(selmax - min) / max * rect.Height()) + 0.5) + rect.top;
				if (!IsActive) {
					pDC->FillSolidRect(CRect(rect.left, rect.top, rect.right, rect.bottom), pDC->GetBkColor());
				}
				else {
					pDC->FillSolidRect(CRect(rect.left, rect.top, rect.right, rect.bottom), GetSysColor(COLOR_WINDOW));
				}
				pDC->FillSolidRect(CRect(rect.left, selmin, rect.right, selmax), hot ? RGB(255, 0, 0) : GetSysColor(COLOR_HIGHLIGHT));
				pDC->ExcludeClipRect(rect);
			}
			else {
				hot = selmax >= 100;
				// fix windows bug
				if (rect.bottom > rect.right) {
					rect.left ^= rect.top ^= rect.left ^= rect.top; // swap left and top values
					rect.right ^= rect.bottom ^= rect.right ^= rect.bottom; // swap right and bottom values
				}
				rect.DeflateRect(2, 2);
				selmin = (int)(((double)(selmin - min) / max * rect.Width()) + 0.5) + rect.left;
				selmax = (int)(((double)(selmax - min) / max * rect.Width()) + 0.5) + rect.left;
				if (!IsActive) {
					pDC->FillSolidRect(CRect(rect.left, rect.top, rect.right, rect.bottom), pDC->GetBkColor());
				}
				else {
					pDC->FillSolidRect(CRect(rect.left, rect.top, rect.right, rect.bottom), GetSysColor(COLOR_WINDOW));
				}
				pDC->FillSolidRect(CRect(selmin, rect.top, selmax, rect.bottom), hot ? RGB(255, 0, 0) : GetSysColor(COLOR_HIGHLIGHT));
				pDC->ExcludeClipRect(rect);
			}
			*result = CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
			break;
		}
		}
		break;
	case CDDS_ITEMPOSTPAINT:
		switch (pNmcd->dwItemSpec) {
		case TBCD_CHANNEL: {
			CDC *pDC = CDC::FromHandle(pNmcd->hdc);
			pDC->SelectClipRgn(NULL);
			break;
		}
		}
		break;
	}
}
