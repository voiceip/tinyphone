// StdioFileEx.cpp: implementation of the CStdioFileEx class.
//
// Version 1.1 23 August 2003.	Incorporated fixes from Dennis Jeryd.
// Version 1.3 19 February 2005. Incorporated fixes from Howard J Oh and some of my own.
// Version 1.4 26 February 2005. Fixed stupid screw-up in code from 1.3.
// Version 1.5 18 November 2005. - Incorporated fixes from Andy Goodwin.
//	                              - Allows code page to be specified for reading/writing
//	                              - Properly calculates multibyte buffer size instead of
//	                                 assuming lstrlen(s).
//	                              - Should handle UTF8 properly.
// Version 1.6 19 July 2007.     - ReadString incorrectly removed \r or \n characters 
//	                                immediately preceding line breaks.
//                                 Fixed tab problem in these comments! (Perry).
//                                 Made GetMultiByteStringFromUnicodeString input string const
//                                  (Perry).
//                                 Avoided double conversion if code page not set.
//                                  (Konrad Windszus).
//                                 Fixed ASSERT in GetUnicodeStringFromMultiByteString
//                                  (Konrad Windszus).
//                                 Maximum line length restriction removed. Lines of any length
//                                   can now be read thanks to C.B. Falconer's fggets (fgoodgets),
//                                   ably assisted by Ana Sayfa and Dave Kondrad.
//                                 Substantial code reorganisation and tidying.
//                                 Use of strlen/lstrlen eliminated. Conversion functions always used
//                                   to calculate required buffers.
//                                 Serious, systematic tests are now included with the code.
//                                 Options included to switch off BOM writing and alter the Unicode
//                                   filler char.
//											  BOM is only stripped off if actually there.
//                                 UTF-8 BOM is now read and written. UTF-8 conversion works.
//
// Copyright David Pritchard 2003-2007. davidpritchard@ctv.es
//
// You can use this class freely, but please keep my ego happy 
// by leaving this comment in place.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StdioFileEx.h"
#include "ggets.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif 

const unsigned char UTF8_BOM[] = { unsigned char(0xEF), unsigned char(0xBB), unsigned char(0xBF) };

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Add this flag to write in Unicode. For the moment, out of range of all the Visual Studio 2005 flags
/*static*/ const UINT CStdioFileEx::modeWriteUnicode = 0x200000;

CStdioFileEx::CStdioFileEx() :
	m_bCheckFilePos(true),
	m_bIsUnicodeText(false),
	m_nFileCodePage(-1),
	m_cUnicodeFillerChar(sDEFAULT_UNICODE_FILLER_CHAR),
	m_bWriteBOM(true), // By default, write the BOM
	CStdioFile()
{
}

// Set the code page for reading/writing
void CStdioFileEx::SetCodePage(IN const UINT nCodePage)
{
	m_nFileCodePage = (int)nCodePage;
}

// Set the Unicode filler char - the char written when no conversion is possible for the target multibyte char set
void CStdioFileEx::SetFillerChar(IN const char cFiller)
{
	m_cUnicodeFillerChar = cFiller;
}

// Determines whether the byte-order-mark is written at the start of a Unicode file
void CStdioFileEx::SetWriteBOM(IN const bool bWrite)
{
	m_bWriteBOM = bWrite;
}

// Determines whether we try to interpret this file as Unicode
//void CStdioFileEx::SetUnicode(IN const bool bIsUnicode)
//{
//	m_bIsUnicodeText = bIsUnicode;
//}

BOOL CStdioFileEx::Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError /*=NULL*/)
{
	// Process any Unicode stuff. This no longer checks for the Unicode BOM. We do this on
	// opening for efficiency.
	ProcessFlags(nOpenFlags);

	BOOL bOK = CStdioFile::Open(lpszFileName, nOpenFlags, pError);

	if (bOK)
	{
		// If we are reading, see if it has a BOM. I tried making the Unicode-ness independent of the BOM (i.e. allowed the file to
		// be identified as Unicode by the caller, with the BOM just being used as a check, or thrown away). 
		// But for some reason it wouldn't work. I'll no doubt try again at some point. 
//		if (!(nOpenFlags & CFile::modeCreate) && (nOpenFlags & CFile::modeRead || nOpenFlags & CFile::modeReadWrite))
		if (!(nOpenFlags & CFile::modeCreate) && !(nOpenFlags & CFile::modeWrite))
		{
			wchar_t	cBOMTest;
			wchar_t	cBOM = nUNICODE_BOM;

			Read(&cBOMTest, sizeof(wchar_t));
			// Reset to start of file
			SeekToBegin();

			// If the first characters are NOT a BOM, reset to start of file
			m_bIsUnicodeText = (wmemcmp(&cBOMTest, &cBOM, 1) == 0);
			if (!m_bIsUnicodeText) {
				char *pChar = (char *)&cBOMTest;
				if (*(pChar + 1) == 0) {
					m_bIsUnicodeText = true;
				}
			}
			m_bCheckFilePos = true;
		}
	}

	return bOK;
}

BOOL CStdioFileEx::ReadString(CString& rString)
{
	ASSERT(m_pStream != NULL);

	BOOL			bReadData = FALSE;
	LPTSTR		lpsz;
	int			nLen = 0;

	// If at position 0, discard byte-order mark before reading. To optimise reading, we only
	// check this when the m_bCheckFilePos is set (this avoids a call to ftell every time we
	// read a line)
	if (m_bCheckFilePos && GetPosition() == 0)
	{
		m_bReadBOM = false;

		// Look for Unicode BOM
		if (m_bIsUnicodeText)
		{
			wchar_t	cBOMTest;
			wchar_t	cBOM = nUNICODE_BOM;
			Read(&cBOMTest, sizeof(wchar_t));
			// If the first characters are NOT a BOM, reset to start of file
			if (wmemcmp(&cBOMTest, &cBOM, 1) != 0)
			{
				SeekToBegin();
				ASSERT(GetPosition() == 0);
			}
			else
			{
				// Set read BOM flag
				m_bReadBOM = true;
			}
		}
		// Look for UTF8 BOM
		else if (CP_UTF8 == m_nFileCodePage)
		{
			BYTE	arrUTF8BOMTest[sizeof(UTF8_BOM)];
			Read(arrUTF8BOMTest, sizeof(arrUTF8BOMTest));

			// If the first characters are NOT a BOM, reset to start of file
			if (memcmp(&arrUTF8BOMTest, UTF8_BOM, sizeof(arrUTF8BOMTest)) != 0)
			{
				SeekToBegin();
				ASSERT(GetPosition() == 0);
			}
			else
			{
				// Set read BOM flag
				m_bReadBOM = true;
			}
		}
	}

	// Read Unicode line or multibyte line (implementations 
	// differ depending on the compilation)
	if (m_bIsUnicodeText)
	{
		bReadData = ReadUnicodeLine(rString);
	}
	else
	{
		bReadData = ReadMultiByteLine(rString);
	}

	// Then remove end-of-line character as necessary.  
	// fggets keeps the end-of-line confusion level at maximum by stripping the \n
	// from the end of lines, but leaving the \r. Grrrr.
	// Remember that you could quite legitimately have a \r or \n at the end of
	// your line before the actual \r\n line break. 
	if (bReadData)
	{
		// Copied from FileTxt.cpp but adapted to use of fgets
		nLen = rString.GetLength();

		lpsz = rString.GetBuffer(0);

		// Strip \r from the end
		if (nLen != 0 && (lpsz[nLen - 1] == _T('\r')))
		{
			rString.GetBufferSetLength(nLen - 1);
		}

		rString.ReleaseBuffer();

		// Now we've moved on in the file, don't bother to check any more unless the
		// file pointer is moved
		m_bCheckFilePos = false;
	}

	return bReadData;
}

/*virtual*/ LPTSTR CStdioFileEx::ReadString(LPTSTR lpsz, UINT nMax)
{
	// Can't really provide compatible behaviour at the moment, so no Unicode handling.
	ASSERT(false);

	return CStdioFile::ReadString(lpsz, nMax);
}


// --------------------------------------------------------------------------------------------
//
//	CStdioFileEx::WriteString()
//
// --------------------------------------------------------------------------------------------
// Returns:    void
// Parameters: LPCTSTR lpsz
//
// Purpose:		Writes string to file either in Unicode or multibyte, depending on whether the caller specified the
//					CStdioFileEx::modeWriteUnicode flag. Override of base class function.
// Notes:		If writing in Unicode we need to:
//						a) Write the Byte-order-mark at the beginning of the file
//						b) Write all strings in byte-mode
//					-	If we were compiled in Unicode, we need to convert Unicode to multibyte if 
//						we want to write in multibyte
//					-	If we were compiled in multi-byte, we need to convert multibyte to Unicode if 
//						we want to write in Unicode.
// Exceptions:	None.
//
void CStdioFileEx::WriteString(LPCTSTR lpsz)
{
	// Write Byte Order Mark if needed
	if (m_bWriteBOM && (!m_pStream || GetPosition() == 0))
	{
		// If writing Unicode and at the start of the file, need to write byte mark
		if (m_nFlags & CStdioFileEx::modeWriteUnicode)
		{
			// If at position 0, write byte-order mark before writing anything else
			wchar_t cBOM = (wchar_t)nUNICODE_BOM;
			CFile::Write(&cBOM, sizeof(wchar_t));
		}
		// Fix by philfar 
		// http://www.codeproject.com/file/stdiofileex.asp?forumid=15472&select=1805024&df=100#xx1805024xx
		// otherwise, if we are writing UTF-8 and at the start of the file, need to write UTF-8 byte mark
		else if (m_nFileCodePage == CP_UTF8)
		{
			// If at position 0, write byte-order mark before writing anything else
			CFile::Write(UTF8_BOM, sizeof(UTF8_BOM));
		}
	}

	// If writing Unicode...
	if (m_nFlags & CStdioFileEx::modeWriteUnicode)
	{
		WriteUnicodeLine(lpsz);
	}
	// Else write multibyte/ANSI
	else
	{
		WriteMultiByteLine(lpsz);
	}
}

UINT CStdioFileEx::ProcessFlags(UINT& nOpenFlags)
{
	m_bIsUnicodeText = false;

	// If we have writeUnicode we must have write or writeRead as well
#ifdef _DEBUG
	if (nOpenFlags & CStdioFileEx::modeWriteUnicode)
	{
		ASSERT(nOpenFlags & CFile::modeWrite || nOpenFlags & CFile::modeReadWrite);
	}
#endif

	// Konrad Windszus 05/04/2006: handle case of writing to Unicode (obviously always Unicode) 
	if (nOpenFlags & CStdioFileEx::modeWriteUnicode)
	{
		m_bIsUnicodeText = true;
	}

	// Always use binary mode, for any type of writing
	if (nOpenFlags & CFile::typeText)
	{
		nOpenFlags ^= CFile::typeText;
	}
	nOpenFlags |= CFile::typeBinary;

	m_nFlags = nOpenFlags;

	return nOpenFlags;
}

// --------------------------------------------------------------------------------------------
//
//	CStdioFileEx::IsFileUnicode()
//
// --------------------------------------------------------------------------------------------
// Returns:    bool
// Parameters: const CString& sFilePath
//
// Purpose:		Determines whether a file is Unicode by reading the first character and detecting
//					whether it's the Unicode byte marker.
// Notes:		None.
// Exceptions:	None.
//
/*static*/ bool CStdioFileEx::IsFileUnicode(const CString& sFilePath)
{
	CFile				file;
	bool				bIsUnicode = false;
	wchar_t			cFirstChar;
	CFileException	exFile;

	// Open file in binary mode and read first character
	if (file.Open(sFilePath, CFile::typeBinary | CFile::modeRead, &exFile))
	{
		// If byte is Unicode byte-order marker, let's say it's Unicode
		if (file.Read(&cFirstChar, sizeof(wchar_t)) > 0 && cFirstChar == (wchar_t)nUNICODE_BOM)
		{
			bIsUnicode = true;
		}

		file.Close();
	}
	else
	{
		// Handle error here if you like
	}

	return bIsUnicode;
}

// Rough character count. Likely to be way out for multibyte files using non-western
// code pages. Anybody got a better method?
STDIOEXLONG CStdioFileEx::GetCharCount()
{
	int			nCharSize;
	ULONGLONG	nByteCount, nCharCount = 0;

	if (m_pStream)
	{
		// Get size of chars in file
		nCharSize = m_bIsUnicodeText ? sizeof(wchar_t) : sizeof(char);

		// If Unicode, remove byte order mark from count
		nByteCount = GetLength();

		if (m_bIsUnicodeText)
		{
			nByteCount = nByteCount - sizeof(wchar_t);
		}

		// Calc chars
		nCharCount = (nByteCount / nCharSize);
	}

	return nCharCount;
}

/*virtual*/ STDIOEXLONG CStdioFileEx::Seek(LONGLONG lOff, UINT nFrom)
{
	// Note that we should check the file position on the next read
	m_bCheckFilePos = true;

	return CStdioFile::Seek(lOff, nFrom);
}


// Get the current users code page
UINT CStdioFileEx::GetCurrentLocaleCodePage()
{
	_TCHAR	szLocalCodePage[10];
	UINT		nLocaleCodePage = 0;
	int		nLocaleChars = ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, szLocalCodePage, 10);

	// If OK
	if (nLocaleChars > 0)
	{
		nLocaleCodePage = (UINT)_ttoi(szLocalCodePage);
		ASSERT(nLocaleCodePage > 0);
	}
	else
	{
		ASSERT(false);
	}

	// O means either: no ANSI code page (Unicode-only locale?) or failed to get locale
	// In the case of Unicode-only locales, what do multibyte apps do? Answers on a postcard.
	return nLocaleCodePage;
}

// --------------------------------------------------------------------------------------------
//
//	CStdioFileEx::GetUnicodeStringFromMultiByteString()
//
// --------------------------------------------------------------------------------------------
// Returns:    int - number of chars written
// Parameters: LPCSTR		szMultiByteString		(IN)	Multi-byte input string
//					wchar_t*		szUnicodeString		(OUT)	Unicode outputstring
//					size_t		nUnicodeBufferSize	(IN)	Size of Unicode output buffer in chars
//					UINT			nCodePage				(IN)	Code page used to perform conversion
//																		Default = CP_ACP (Get local code page).
//
// Purpose:		Gets a Unicode string from a MultiByte string.
// Notes:		None.
// Exceptions:	None.
//
int CStdioFileEx::GetUnicodeStringFromMultiByteString(IN LPCSTR szMultiByteString, OUT wchar_t* szUnicodeString, IN const size_t nUnicodeBufferSize, IN UINT nCodePage)
{
	int		nCharsWritten = 0;

	if (szUnicodeString && szMultiByteString)
	{
		// If no code page specified, take default for system
		if (nCodePage == (UINT)-1)
		{
			nCodePage = GetACP();
		}

		try
		{
			// Zero out buffer first
			memset((void*)szUnicodeString, '\0', sizeof(wchar_t) * nUnicodeBufferSize);

			// When converting to UTF8, don't set any flags (see Q175392).
			nCharsWritten = MultiByteToWideChar((UINT)nCodePage, (nCodePage == CP_UTF8 ?
				0 : MB_PRECOMPOSED), szMultiByteString, -1, szUnicodeString, (int)nUnicodeBufferSize);
		}
		catch (...)
		{
			// Level 4 compile says this is unreachable code in VS2005. I guess MultiByteToWideChar doesn't throw exceptions.
//			TRACE(_T("Controlled exception in MultiByteToWideChar!\n"));
		}
	}
	else
	{
		ASSERT(false);
	}

	// Konrad Windszus 29/3/2006: ASSERT in wrong place!
	// Should have at least the terminator (right?)
	ASSERT(nCharsWritten > 0);

	// Now fix nCharsWritten to exclude \0 terminator 
	if (nCharsWritten > 0)
	{
		nCharsWritten--;
	}

	return nCharsWritten;
}

// --------------------------------------------------------------------------------------------
//
//	CStdioFileEx::GetRequiredUnicodeLengthFromMultiByteString()
//
// --------------------------------------------------------------------------------------------
// Returns:    int - number of chars needed
// Parameters: LPCSTR		szMultiByteString		(IN)	Multi-byte input string
//					UINT			nCodePage				(IN)	Code page of input string
//																		Default = CP_ACP (local code page).
//
// Purpose:		Gets the length required, in wchar_t values (chars) to convert a MultiByte string to a Unicode string.
// Notes:		None.
// Exceptions:	None.
//
/*static*/ int CStdioFileEx::GetRequiredUnicodeLengthFromMultiByteString(IN LPCSTR szMultiByteString, IN UINT nCodePage /*=CP_ACP OPTIONAL*/)
{
	int	nCharsNeeded = 0;

	if (szMultiByteString)
	{
		// If no code page specified, take default for system
		if (nCodePage == (UINT)-1)
		{
			nCodePage = GetACP();
		}

		try
		{
			// When converting to UTF8, don't set any flags (see Q175392).
			nCharsNeeded = MultiByteToWideChar((UINT)nCodePage, (nCodePage == CP_UTF8 ?
				0 : MB_PRECOMPOSED),
				szMultiByteString,
				-1,	// Get the function to work out the length
				NULL, // No output buffer needed
				0		// Ask to be told how many chars we need (includes space for terminator)
			);
		}
		catch (...)
		{
			// Level 4 compile says this is unreachable code in VS2005. I guess MultiByteToWideChar doesn't throw exceptions.
//			TRACE(_T("Controlled exception in MultiByteToWideChar!\n"));
		}
	}
	else
	{
		ASSERT(false);
	}

	// Konrad Windszus 29/3/2006: ASSERT in wrong place!
	// Should have at least the terminator (right?). The value returned by MultiByteToWideChar includes the terminator.
	ASSERT(nCharsNeeded > 0);

	return nCharsNeeded;
}

// --------------------------------------------------------------------------------------------
//
//	CStdioFileEx::GetNewUnicodeStringFromMultiByteString()
//
// --------------------------------------------------------------------------------------------
// Returns:    int - number of chars written (0 means error)
// Parameters: LPCSTR		szMultiByteString		(IN)		Multi-byte input string
//					CTemplateSmartPtrArray<wchar_t>& 
//									spUnicodeString		(IN/OUT)	Smart pointer containing default buffer (or NULL) 
//																				on input, and pointing to buffer used for conversion
//																				on output. A newly allocated buffer will be automatically
//																				deleted when the smart ptr object is destroyed.
//																				This allows a default buffer to be declared and used for
//																				most strings. Dynamic allocation is only performed when
//																				the default buffer would not be large enough.
//					int			nDefaultBufferSize	(IN)		Size of default buffer in smart ptr (may be 0).
//					UINT			nCodePage				(IN)		Code page used to perform conversion
//																			Default = CP_ACP (Get local code page).
//
// Purpose:		Gets a Unicode string from a MultiByte string. Calculates the buffer for you and
//					allocates it with "new". 
// Notes:		It's better to ask this function to allocate the buffer for you, because it will
//					calculate the correct size. If we just take the number of bytes from the multibyte
//					string as the size, we won't be in danger of allocating too little memory, but we
//					may well allocate too much.
//					
//					The use of a smart ptr array combines this flexibility with efficiency. A default buffer can be passed in
//					and used wherever is it sufficient to contain the output string. This avoids lots of unnecessary "new"s and
//					"delete"s when reading or writing large files.
// Exceptions:	None.
//
/*static*/ int CStdioFileEx::GetNewUnicodeStringFromMultiByteString(IN LPCSTR szMultiByteString, IN OUT CTemplateSmartPtrArray<wchar_t>& spUnicodeString, IN const int nDefaultBufferSize/*=0*/, IN UINT nCodePage /*=CP_ACP OPTIONAL*/)
{
	int nActualBufferSize = nDefaultBufferSize;

	// Calculate the required buffer size and allocate
	int nUnicodeBufferSizeChars = GetRequiredUnicodeLengthFromMultiByteString(szMultiByteString, nCodePage);

	// If we have enough in the default buffer, don't bother to allocate
	if (nUnicodeBufferSizeChars > nDefaultBufferSize)
	{
		//pszUnicodeString = new wchar_t[nUnicodeBufferSizeChars];

		// The new buffer will be marked as "owned" by the smart ptr by default, and therefore automatically deleted by the smart ptr
		spUnicodeString.Assign(new wchar_t[nUnicodeBufferSizeChars]);

		nActualBufferSize = nUnicodeBufferSizeChars;
	}

	// Call standard
	return GetUnicodeStringFromMultiByteString(szMultiByteString, spUnicodeString.GetBuffer(), nActualBufferSize, nCodePage);
}

// --------------------------------------------------------------------------------------------
//
//	CStdioFileEx::GetMultiByteStringFromUnicodeString()
//
// --------------------------------------------------------------------------------------------
// Returns:    int	- number of chars written. 0 if error.
// Parameters: wchar_t *	szUnicodeString			(IN)	Unicode input string
//					char*			szMultiByteString			(OUT)	Multibyte output string
//					int			nMultiByteBufferSize		(IN)	Multibyte buffer size
//					UINT			nCodePage					(IN)	Code page used to perform conversion
//																			Default = CP_ACP (Get local code page).
//					char			cFillerChar					(IN)  Unicode-to-multibyte filler char 
//																			Default = #
//
// Purpose:		Gets a MultiByte string from a Unicode string.
// Notes:		It's better to ask this function to allocate the buffer for you, because it will
//					calculate the correct size. Multibyte code pages will require larger buffers than
//					the normal Western code pages, so we can't just say new char[numchars]!
// Exceptions:	None.
//
int CStdioFileEx::GetMultiByteStringFromUnicodeString(IN const wchar_t * szUnicodeString, OUT char* szMultiByteString,
	IN const int nMultiByteBufferSize, IN UINT nCodePage/*=CP_ACP OPTIONAL*/,
	IN char cFillerChar/*=sDEFAULT_UNICODE_FILLER_CHAR OPTIONAL*/)
{
	BOOL			bUsedDefChar = FALSE;
	int			nBytesWritten = 0;

	// Fix by Andy Goodwin: don't do anything if buffer is 0
	if (nMultiByteBufferSize > 0)
	{
		if (szUnicodeString && szMultiByteString)
		{
			// Zero out buffer first
			memset((void*)szMultiByteString, '\0', sizeof(char) * nMultiByteBufferSize);

			// If no code page specified, take default for system
			if (nCodePage == (UINT)-1)
			{
				nCodePage = (UINT)GetACP();
			}

			try
			{
				// If writing to UTF8, flags, default char and boolean flag must be NULL
				nBytesWritten = WideCharToMultiByte((UINT)nCodePage,
					(nCodePage == CP_UTF8 ? 0 : WC_COMPOSITECHECK | WC_SEPCHARS), // Flags
					szUnicodeString, -1,
					szMultiByteString,
					nMultiByteBufferSize,
					(nCodePage == CP_UTF8 ? NULL : &cFillerChar),		// Filler char
					(nCodePage == CP_UTF8 ? NULL : &bUsedDefChar));		// Did we use filler char?

				// If no chars were written and the buffer is not 0, error!
				if (nBytesWritten == 0 && nMultiByteBufferSize > 0)
				{
					TRACE1("Error in WideCharToMultiByte: %d\n", ::GetLastError());
				}
			}
			catch (...)
			{
				TRACE0("Controlled exception in WideCharToMultiByte!\n");
			}
		}
	}

	// Now fix nCharsWritten to exclude \0 terminator 
	if (nBytesWritten > 0)
	{
		nBytesWritten = nBytesWritten - sizeof('\0');
	}

	return nBytesWritten;
}



//---------------------------------------------------------------------------------------------------
//
//	CStdioFileEx::GetRequiredMultiByteLengthForUnicodeString()
//
//---------------------------------------------------------------------------------------------------
// Returns:    int - no of bytes required
// Parameters: wchar_t *	szUnicodeString			(IN)	String to convert
//					UINT			nCodePage=CP_ACP			(IN)	Code page to which to convert
//
// Purpose:		Obtains the multi-byte buffer size (in bytes) needed to accommodate a converted Unicode string.
//	Notes:		We can't assume that the buffer length is simply equal to the number of characters
//					because that wouldn't accommodate multibyte characters!
//
/*static*/ int CStdioFileEx::GetRequiredMultiByteLengthForUnicodeString(IN const wchar_t * szUnicodeString, IN UINT nCodePage /*=CP_ACP OPTIONAL*/)
{
	int nBytesNeeded = 0;

	try
	{
		// If no code page specified, take default for system
		if (nCodePage == -1)
		{
			nCodePage = GetACP();
		}

		// If writing to UTF8, flags, default char and boolean flag must be NULL
		nBytesNeeded = WideCharToMultiByte((UINT)nCodePage,
			(nCodePage == CP_UTF8 ? 0 : WC_COMPOSITECHECK | WC_SEPCHARS), // Flags
			szUnicodeString, -1,
			NULL,
			0,	// Calculate required buffer, please! (Includes space for terminator)
			NULL,	// Filler char doesn't matter here
			NULL);
	}
	catch (...)
	{
		// TRACE(_T("Controlled exception in WideCharToMultiByte!\n"));
		// Gives us "unreachable code" error compiling on level 4
	}

	return nBytesNeeded;
}


// --------------------------------------------------------------------------------------------
//
//	CStdioFileEx::GetNewMultiByteStringFromUnicodeString()
//
// --------------------------------------------------------------------------------------------
// Returns:    int	- number of chars written. 0 if error.
// Parameters: wchar_t *	szUnicodeString			(IN)		Unicode input string
//					CTemplateSmartPtrArray<char>& 
//									spMultiByteString			(IN/OUT)	Smart pointer containing default buffer (or NULL) 
//																					on input, and pointing to buffer used for conversion
//																					on output. A newly allocated buffer will be automatically
//																					deleted when the smart ptr object is destroyed.
//																					This allows a default buffer to be declared and used for
//																					most strings. Dynamic allocation is only performed when
//																					the default buffer would not be large enough.
//					int			nDefaultBufferSize		(IN)		Size of default buffer in smart ptr (may be 0).
//					UINT			nCodePage					(IN)		Code page used to perform conversion
//																					Default = CP_ACP (Get local code page).
//					char			cFillerChar					(IN)		Unicode-to-multibyte filler char 
//																				Default = #
//
// Purpose:		Gets a MultiByte string from a Unicode string. Calculates the buffer for you and
//					allocates it with new. 
// Notes:		It's better to ask this function to allocate the buffer for you, because it will
//					calculate the correct size. Multibyte code pages will require larger buffers than
//					the normal Western code pages, so we can't just say new char[numchars]!
//					
//					The use of a smart ptr array combines this flexibility with efficiency. A default buffer can be passed in
//					and used wherever is it sufficient to contain the output string. This avoids lots of unnecessary "new"s and
//					"delete"s when reading or writing large files.
// Exceptions:	None.
//
/*static*/ int CStdioFileEx::GetNewMultiByteStringFromUnicodeString(IN const wchar_t * szUnicodeString, IN OUT CTemplateSmartPtrArray<char>& spMultiByteString, IN const int nDefaultBufferSize/*=0*/,
	IN UINT nCodePage /*=CP_ACP OPTIONAL*/, IN char cFillerChar/*=sDEFAULT_UNICODE_FILLER_CHAR OPTIONAL*/)
{
	int nActualBufferSize = nDefaultBufferSize;

	// Calculate the required buffer size and allocate
	int nMultibyteBufferSizeBytes = GetRequiredMultiByteLengthForUnicodeString(szUnicodeString, nCodePage);

	// If we have enough in the default buffer, don't bother to allocate
	if (nMultibyteBufferSizeBytes > nDefaultBufferSize)
	{
		// The new buffer will be marked as "owned" by the smart ptr by default, and therefore automatically deleted by the smart ptr
		spMultiByteString.Assign(new char[nMultibyteBufferSizeBytes]);

		nActualBufferSize = nMultibyteBufferSizeBytes;
	}

	// Call standard
	return GetMultiByteStringFromUnicodeString(szUnicodeString, spMultiByteString.GetBuffer(), nActualBufferSize, nCodePage, cFillerChar);
}

//---------------------------------------------------------------------------------------------------
//
//	CStdioFileEx::GetNewUTF8StringFromUnicodeString()
//
//---------------------------------------------------------------------------------------------------
// Returns:    bool - true if successful, false if it fails.
// Parameters: const wchar_t*		szUnicodeString		(IN)	Input Unicode string
//					unsigned char*&	pszUTF8String			(OUT) Receives a ptr. If the function returns
//																				successfully.
//																				the ptr points to the output string
//
// Purpose:		Does conversion from Unicode to UTF8. Allocates memory for the output string
//	Notes:		Culled from http://www.bytemycode.com/snippets/snippet/438/
//					Contributed by Dean
//
/*static*/ /*bool CStdioFileEx::GetNewUTF8StringFromUnicodeString( IN const wchar_t* szUnicodeString, OUT unsigned char*& pszUTF8String )
{
	bool	bConvertedOK = false;
	const wchar_t* w;

	// Calculate length needed for output string, taking account
	// of the variable number of bytes needed to represent each
	// Unicode character in UTF8
	int len = 0;
	for ( w = szUnicodeString; *w; w++ )
	{
		if ( *w < 0հ080 ) len++;

		else if ( *w < 0հ800 ) len += 2;

		else len += 3;
	}

	//unsigned char* szOut = ( unsigned char* )malloc( len+1 );
	pszUTF8String = new char[len+1];

	if ( pszUTF8String != NULL )
	{
		int i = 0;

		for ( w = szUnicodeString; *w; w++ )
		{
			// Handle ASCII chars
			if ( *w < 0հ080 )
			{
				pszUTF8String[i++] = ( char ) *w;
			}
			else if ( *w < 0հ800 )
			{
				pszUTF8String[i++] = 0xc0 | (( *w ) >> 6 );
				pszUTF8String[i++] = 0ո0 | (( *w ) & 0ճf );
			}
			else
			{
				pszUTF8String[i++] = 0xe0 | (( *w ) >> 12 );
				pszUTF8String[i++] = 0ո0 | (( ( *w ) >> 6 ) & 0ճf );
				pszUTF8String[i++] = 0ո0 | (( *w ) & 0ճf );
			}
		}

		pszUTF8String[ i ] = \0;

		bConvertedOK = true;
	}

	//return ( char* )szOut;
	return bConvertedOK;
}*/

/**********************************************************************************/
/*                                  Reading                                       */
/**********************************************************************************/
#ifdef _UNICODE
// Read Unicode in Unicode compilation
BOOL CStdioFileEx::ReadUnicodeLine(OUT CString& sOutputLine)
{
	BOOL			bReadData = FALSE;
	wchar_t*		pszUnicodeString = (wchar_t*)&m_arrUnicodeDefaultBuffer;
	bool			bNeedToDelete = false;

	try
	{
		bReadData = (FGGETS_OK == fggets<wchar_t>(&pszUnicodeString, m_pStream, bNeedToDelete, nSTDIOFILEEX_DEFAULT_BUFFER_SIZE));

		if (bReadData)
		{
			sOutputLine = (CString)pszUnicodeString;
		}
		else
		{
			sOutputLine.Empty();
		}
	}
	// Ensure we always clean up, no matter what
	FINALLY(if (bNeedToDelete) DELETE_SAFE_ARRAY(pszUnicodeString); )

		return bReadData;
}

// Read Multibyte in Unicode compilation
BOOL CStdioFileEx::ReadMultiByteLine(OUT CString& sOutputLine)
{
	BOOL			bReadData = FALSE;
	char	*		pszMultiByteString = (char*)&m_arrMultibyteDefaultBuffer;;
	int			nChars = 0;
	bool			bNeedToDelete = false;

	try
	{
		// Read the string -- the function dynamically allocates the necessary memory according
		// to the line length
		bReadData = (FGGETS_OK == fggets<char>(&pszMultiByteString, m_pStream, bNeedToDelete, nSTDIOFILEEX_DEFAULT_BUFFER_SIZE));

		if (bReadData)
		{
			// Assign default buffer to take care of 99% of cases. Not owned by smart ptr array, so won't be deleted
			CTemplateSmartPtrArray<wchar_t>	spUnicodeString((wchar_t*)&m_arrUnicodeDefaultBuffer, bTEMPLATESMARTPTR_NOTOWNED);

			// Use all-in-one allocation and conversion function. Avoid _mbslen and such like since they're unreliable
			// with UTF8
			nChars = GetNewUnicodeStringFromMultiByteString(pszMultiByteString, spUnicodeString, nSTDIOFILEEX_DEFAULT_BUFFER_SIZE, m_nFileCodePage);

			if (nChars > 0)
			{
				sOutputLine = (CString)spUnicodeString.GetBuffer();
			}
		}

		// Empty the string if we failed to read anything
		if (nChars == 0)
		{
			sOutputLine.Empty();
		}
	}
	// Ensure we always clean up, no matter what
	FINALLY(if (bNeedToDelete) DELETE_SAFE_ARRAY(pszMultiByteString); )

		return bReadData;
}

#else

// Read Unicode in Multibyte compilation
BOOL CStdioFileEx::ReadUnicodeLine(OUT CString& sOutputLine)
{
	BOOL			bReadData = FALSE;
	wchar_t*		pszUnicodeString = (wchar_t*)&m_arrUnicodeDefaultBuffer;
	bool			bNeedToDelete = false;
	int			nChars = 0;

	try
	{
		bReadData = (FGGETS_OK == fggets<wchar_t>(&pszUnicodeString, m_pStream, bNeedToDelete, nSTDIOFILEEX_DEFAULT_BUFFER_SIZE));

		if (bReadData)
		{
			// Assign default buffer to take care of 99% of cases. Not owned by smart ptr array, so won't be deleted
			CTemplateSmartPtrArray<char>	spMultiByteString((char*)&m_arrMultibyteDefaultBuffer, bTEMPLATESMARTPTR_NOTOWNED);

			// Call all-in-one function to calculate required buffer size and allocate
			nChars = GetNewMultiByteStringFromUnicodeString(pszUnicodeString, spMultiByteString, nSTDIOFILEEX_DEFAULT_BUFFER_SIZE, m_nFileCodePage, m_cUnicodeFillerChar);

			if (nChars > 0)
			{
				sOutputLine = (CString)spMultiByteString.GetBuffer();
			}
		}

		// Empty the string if we failed to read anything
		if (nChars == 0)
		{
			sOutputLine.Empty();
		}
	}
	// Ensure we always clean up, no matter what
	FINALLY(if (bNeedToDelete) DELETE_SAFE_ARRAY(pszUnicodeString); )

		return bReadData;
}

// Read Multibyte in Multibyte compilation
BOOL CStdioFileEx::ReadMultiByteLine(OUT CString& sOutputLine)
{
	BOOL			bReadData = FALSE;
	char	*		pszMultiByteString = (char*)&m_arrMultibyteDefaultBuffer;;
	int			nChars = 0;
	UINT			nLocaleCodePage = 0;
	bool			bNeedToDelete = false;

	try
	{
		// Read multibyte from file
		bReadData = (FGGETS_OK == fggets<char>(&pszMultiByteString, m_pStream, bNeedToDelete, nSTDIOFILEEX_DEFAULT_BUFFER_SIZE));

		if (bReadData)
		{
			// Convert to CString
			sOutputLine = (CString)pszMultiByteString;

			// Now see if we've got to convert to another code page. Get the current code page
			nLocaleCodePage = GetCurrentLocaleCodePage();

			// If we got it OK...
			if (nLocaleCodePage > 0)
			{
				// If file code page does not match the system code page (and we have a code page!), 
				// we need to do a double conversion!
				// Konrad Windszus 29/3/2006: Do nothing if we haven't set a code page
				if (m_nFileCodePage > 0 && nLocaleCodePage != (UINT)m_nFileCodePage)
				{
					// Assign default buffer to take care of 99% of cases. Not owned by smart ptr array, so won't be deleted
					CTemplateSmartPtrArray<wchar_t>	spUnicodeString((wchar_t*)&m_arrUnicodeDefaultBuffer, bTEMPLATESMARTPTR_NOTOWNED);

					// Use all-in-one allocation and conversion function. Avoid _mbslen and such like since they're unreliable
					// with UTF8
					nChars = GetNewUnicodeStringFromMultiByteString(sOutputLine, spUnicodeString, nSTDIOFILEEX_DEFAULT_BUFFER_SIZE, m_nFileCodePage);

					// Convert back to multibyte using the system code page
					// (This doesn't really confer huge advantages except to avoid "mangling" of non-convertible special
					// characters. So, if a file in the E.European code page is displayed on a system using the 
					// western European code page, special accented characters which the system cannot display will be
					// replaced by the default character (a hash or something), rather than being incorrectly mapped to
					// other, western European accented characters).
					if (nChars > 0)
					{
						// Assign default buffer to take care of 99% of cases. Not owned by smart ptr array, so won't be deleted
						CTemplateSmartPtrArray<char>	spMultiByteString((char*)&m_arrMultibyteDefaultBuffer, bTEMPLATESMARTPTR_NOTOWNED);

						// Call all-in-one function to calculate required buffer size and allocate
						nChars = GetNewMultiByteStringFromUnicodeString(spUnicodeString.GetBuffer(), spMultiByteString, nSTDIOFILEEX_DEFAULT_BUFFER_SIZE, nLocaleCodePage, m_cUnicodeFillerChar);

						sOutputLine = (CString)spMultiByteString.GetBuffer();
					}
				}
			}
		}

		// Empty the string if we failed to read anything
		if (!bReadData)
		{
			sOutputLine.Empty();
		}

	}
	// Ensure we always clean up, no matter what
	FINALLY(if (bNeedToDelete) DELETE_SAFE_ARRAY(pszMultiByteString); )

		return bReadData;
}
#endif

/**********************************************************************************/
/*                                  Writing                                       */
/**********************************************************************************/
#ifdef _UNICODE

// Unicode in Unicode -- no conversion needed
void CStdioFileEx::WriteUnicodeLine(IN LPCTSTR sInputLine)
{
	// Write in byte mode
	CFile::Write(sInputLine, (UINT)(wcslen(sInputLine) * sizeof(wchar_t)));
}

// Multibyte/ANSI in Unicode -- have to convert to Multibyte, taking into account the desired code page
void CStdioFileEx::WriteMultiByteLine(IN LPCTSTR sInputLine)
{
	int			nCharsWritten = 0;

	// Assign default buffer to take care of 99% of cases. Not owned by smart ptr array, so won't be deleted
	CTemplateSmartPtrArray<char>	spMultiByteString((char*)&m_arrMultibyteDefaultBuffer, bTEMPLATESMARTPTR_NOTOWNED);

	// Call all-in-one function to calculate required buffer size and allocate
	nCharsWritten = GetNewMultiByteStringFromUnicodeString(sInputLine, spMultiByteString, nSTDIOFILEEX_DEFAULT_BUFFER_SIZE, m_nFileCodePage, m_cUnicodeFillerChar);

	if (nCharsWritten > 0)
	{
		// Do byte-mode write using actual chars written (fix by Howard J Oh)
		CFile::Write((const void*)spMultiByteString.GetBuffer(),
			nCharsWritten * sizeof(char));
	}
}

#else
// Writing Unicode in Multibyte, need to convert to Unicode
void CStdioFileEx::WriteUnicodeLine(IN LPCTSTR sInputLine)
{
	int			nCharsWritten = 0;

	// Assign default buffer to take care of 99% of cases. Not owned by smart ptr array, so won't be deleted
	CTemplateSmartPtrArray<wchar_t>	spUnicodeString((wchar_t*)&m_arrUnicodeDefaultBuffer, bTEMPLATESMARTPTR_NOTOWNED);

	// Use all-in-one allocation and conversion function. Avoid _mbslen and such like since they're unreliable
	// with UTF8
	nCharsWritten = GetNewUnicodeStringFromMultiByteString(sInputLine, spUnicodeString, nSTDIOFILEEX_DEFAULT_BUFFER_SIZE, m_nFileCodePage);

	if (nCharsWritten > 0)
	{
		// Do byte-mode write using actual chars written (fix by Howard J Oh)
		CFile::Write((const void*)spUnicodeString.GetBuffer(), nCharsWritten * sizeof(wchar_t));
	}
	else
	{
		ASSERT(false);
	}

	// If a buffer was dynamically allocated, it gets deleted by the smart ptr, even on an exception
}

// Writing Multibyte in Multibyte, no conversion needed, unless the code page differs
void CStdioFileEx::WriteMultiByteLine(IN LPCTSTR sInputLine)
{
	UINT			nLocaleCodePage = 0;
	int			nUnicodeCharsWritten = 0;
	int			nMultiByteCharsWritten = 0;

	// Get the current code page
	nLocaleCodePage = GetCurrentLocaleCodePage();

	// if file code page does not match the system code page (and we have a code page!), 
	// we need to do a double conversion!
	// Konrad Windszus 29/3/2006: Do nothing if we haven't set a code page
	if (nLocaleCodePage > 0 && m_nFileCodePage > 0 && nLocaleCodePage != (UINT)m_nFileCodePage)
	{
		// Assign default buffer to take care of 99% of cases. Not owned by smart ptr array, so won't be deleted
		CTemplateSmartPtrArray<wchar_t>	spUnicodeString((wchar_t*)&m_arrUnicodeDefaultBuffer, bTEMPLATESMARTPTR_NOTOWNED);

		// Use all-in-one allocation and conversion function. Avoid _mbslen and such like since they're unreliable
		// with UTF8
		nUnicodeCharsWritten = GetNewUnicodeStringFromMultiByteString(sInputLine, spUnicodeString, nSTDIOFILEEX_DEFAULT_BUFFER_SIZE, nLocaleCodePage);

		// Convert back to multibyte using the file code page
		// (Note that you can't reliably read a non-Unicode file written in code page A on a system using a code page B,
		// modify the file and write it back using code page A, unless you disable all this double-conversion code.
		// In effect, you have to choose between a mangled character display and mangled file writing).
		if (nUnicodeCharsWritten > 0)
		{
			// Assign default buffer to take care of 99% of cases. Not owned by smart ptr array, so won't be deleted
			CTemplateSmartPtrArray<char>	spMultiByteString((char*)&m_arrMultibyteDefaultBuffer, bTEMPLATESMARTPTR_NOTOWNED);

			// Call all-in-one function to calculate required buffer size and allocate
			nMultiByteCharsWritten = GetNewMultiByteStringFromUnicodeString(
				spUnicodeString.GetBuffer(), spMultiByteString, nSTDIOFILEEX_DEFAULT_BUFFER_SIZE, m_nFileCodePage, m_cUnicodeFillerChar);

			// Do byte-mode write. This avoids annoying "interpretation" of \n's as \r\n
			CFile::Write((const void*)spMultiByteString.GetBuffer(), nMultiByteCharsWritten * sizeof(char));

			// If a Unicode buffer was dynamically allocated, it now gets deleted by the smart ptr, even on an exception
		}

		// If a multibyte buffer was dynamically allocated, it now gets deleted by the smart ptr, even on an exception
	}
	else
	{
		// Do byte-mode write. This avoids annoying "interpretation" of \n's as \r\n
		// Use strlen because we want a straightforward byte-for-byte copy. Characters are irrelevant. 
		// strlen always counts bytes. 
		CFile::Write((const void*)sInputLine, strlen((const char*)sInputLine) * sizeof(char));
	}
}

#endif
