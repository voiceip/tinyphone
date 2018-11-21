#pragma once

#include "const.h"

#if defined( UNICODE ) && !defined( _UNICODE )
	#define _UNICODE
#endif

#define SIZEOF(X) (sizeof(X)/sizeof(X[0]))

struct LangPackEntry {
	unsigned linePos;
	DWORD englishHash;
	char *english;	  //not currently used, the hash does everything
	char *local;
	wchar_t *wlocal;
};

struct LangPackStruct {
	TCHAR filename[MAX_PATH];
	char  language[64];
	char  lastModifiedUsing[64];
	char  authors[256];
	char  authorEmail[128];
	bool rtl;
	struct LangPackEntry *entry;
	int entryCount;
	LCID localeID;
	UINT defaultANSICp;
};
extern LangPackStruct langPack;

void LoadLangPackModule(void);
void UnloadLangPackModule(void);
int TranslateDialog(HWND hwndDlg);
void TranslateMenu(HMENU hmenu);

char* LangPackTranslateString(const char *szEnglish, const int W);
__inline LPTSTR Translate(LPTSTR source)
{
#ifdef _UNICODE
	return ( LPTSTR )LangPackTranslateString( (char*)source, 1 );
#else
	return ( LPTSTR )LangPackTranslateString( source, 0 );
#endif

	
}