#include "StdAfx.h"
#include "ModelessMessageBox.h"

void ModelessMessageBox( HWND hWnd /*= NULL*/, LPCTSTR lpText /*= L""*/, LPCTSTR lpCaption /*= L""*/, UINT uType /*= MB_OK */ )
{
	_ModelessMessageBox::Show(hWnd /*= NULL*/,
		lpText/* = L""*/,
		lpCaption /*= L""*/,
		uType/* = MB_OK*/
		);
}