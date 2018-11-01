#pragma once

/********************************************************************
created: 2008/11/23
created: 23:11:2008 11:16
filename: ModelessMessageBox.h
file path:
file base: ModelessMessageBox
file ext: h
author: Shay

purpose: Creating a message box not on the stack, without
Stopping the code flow.
Using thread and message box data on the heap.
usage: ModelessMessageBox(NULL, sEvent, L"Debug events form menu", MB_OK);

*********************************************************************/
class _ModelessMessageBox
{
protected:
	struct MessageData
	{
		HWND hWnd;
		CString lpText;
		CString lpCaption;
		UINT uType;
	};
	static DWORD WINAPI DoMessageBoxThreadProc(LPVOID lpParameter)
	{
		if(lpParameter == NULL)
			return 1L;
		MessageData * pData= static_cast<MessageData *>(lpParameter);
		if (NULL == pData )
			return 1L;
		//else
		::MessageBox(pData->hWnd, pData->lpText, pData->lpCaption, pData->uType);
		delete pData;
		return 0L;

	}

public:

	static void Show( HWND hWnd = NULL,
		LPCTSTR lpText = _T(""),
		LPCTSTR lpCaption = _T(""),
		UINT uType = MB_OK
		)
	{

		MessageData * pMessageData = new MessageData;

		pMessageData->hWnd= hWnd;
		pMessageData->lpText = lpText;
		pMessageData->lpCaption = lpCaption;
		pMessageData->uType = uType;
		LPDWORD threadId = 0;
		CreateThread(NULL, 0, DoMessageBoxThreadProc, pMessageData/*(LPVOID)&m_messageData*/, 0, threadId);
	}

};

void ModelessMessageBox(HWND hWnd = NULL,
						LPCTSTR lpText = _T(""),
						LPCTSTR lpCaption = _T(""),
						UINT uType = MB_OK
						);
