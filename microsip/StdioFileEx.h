// StdioFileEx.h: interface for the CStdioFileEx class.
//
// Version 1.1 23 August 2003.	Incorporated fixes from Dennis Jeryd.
// Version 1.3 19 February 2005. Incorporated fixes from Howard J Oh and some of my own.
// Version 1.4 26 February 2005. Fixed stupid screw-up in code from 1.3.
// Version 1.5 18 November 2005. - Incorporated fixes from Andy Goodwin.
//	                              - Allows code page to be specified for reading/writing
//	                              - Properly calculates multibyte buffer size instead of
//	                              	assuming lstrlen(s).
//	                              - Should handle UTF8 properly.
// Version 1.6 ??? 2006.			- ReadString incorrectly removed \r or \n characters 
//	                                immediately preceding line breaks
//                                 Fixed tab problem in these comments! (Perry)
//                                 Made GetMultiByteStringFromUnicodeString input string const
//                                  (Perry)
//                                 Avoided double conversion if code page not set.
//                                  (Konrad Windszus)
//                                 Fixed ASSERT in GetUnicodeStringFromMultiByteString
//                                  (Konrad Windszus)
//                                 Maximum line length restriction removed. Lines of any length
//                                   can now be read thanks to C.B. Falconer's fggets (fgoodgets),
//                                   ably assisted by Ana Sayfa and Dave Kondrad.
//                                 Substantial code reorganisation and tidying.
//                                 _mbstrlen now used everywhere instead of strlen/lstrlen 
//                                   to get correct multibyte string length.
//                                 Serious, systematic tests are now included with the code.
//											  BOM is only stripped off if actually there.
//                                 UTF-8 BOM is now read and written. UTF-8 conversion works.
//
// Copyright David Pritchard 2003-2007. davidpritchard@ctv.es
//
// You can use this class freely, but please keep my ego happy 
// by leaving this comment in place.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STDIOFILEEX_H__41AFE3CA_25E0_482F_8B00_C40775BCDB81__INCLUDED_)
#define AFX_STDIOFILEEX_H__41AFE3CA_25E0_482F_8B00_C40775BCDB81__INCLUDED_

#include "TemplateSmartPtr.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	MS_COMPILER_VS2005	1400		// v8.0

// Make length type ULONGLONG for VS 2005
#if _MSC_VER < MS_COMPILER_VS2005  // If prior to VS 2005
	typedef unsigned long STDIOEXLONG;
#else
	typedef ULONGLONG STDIOEXLONG;
#endif 

#define	nUNICODE_BOM							0xFEFF		// Unicode "byte order mark" which goes at start of file
#define	sNEWLINE									_T("\r\n")	// New line characters
#define	sDEFAULT_UNICODE_FILLER_CHAR		'#'			// Filler char used when no conversion from Unicode to local code page is possible
#define	nSTDIOFILEEX_DEFAULT_BUFFER_SIZE	4096			// Size of default buffer for strings

// Macro to do post-exception cleanup. Rethrows exception afterwards
#define	FINALLY(x)		catch(...)\
                        {\
                           ASSERT(false);\
                           x;\
                           throw;\
                        }\
                        x;

// An adaptation of the supremely handy DELETE_SAFE macro by mandhjo
// (http://www.experts-exchange.com/Programming/Programming_Languages/MFC/Q_10292771.html)
#define	DELETE_SAFE_ARRAY(parr)		if (parr)\
                                 {\
                                   delete [] parr;\
                                   parr = NULL;\
                                 }

class CStdioFileEx: public CStdioFile
{
public:
	CStdioFileEx();

	virtual BOOL			Open( LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL );
	virtual BOOL			ReadString(CString& rString);
	virtual LPTSTR			ReadString(LPTSTR lpsz, UINT nMax);
	virtual void			WriteString( LPCTSTR lpsz );
	STDIOEXLONG				GetCharCount();
	virtual STDIOEXLONG	Seek(LONGLONG lOff, UINT nFrom);

	bool					IsFileUnicodeText()	{ return m_bIsUnicodeText; }	

	// Additional flag to allow Unicode text writing
	static const UINT modeWriteUnicode;

	void				SetCodePage(IN const UINT nCodePage);
	void				SetFillerChar(IN const char cFiller);
	void				SetWriteBOM(IN const bool bWrite);
	//void				SetUnicode(IN const bool bIsUnicode);

	// static utility functions

	// --------------------------------------------------------------------------------------------
	//
	//	CStdioFileEx::GetUnicodeStringFromMultiByteString()
	//
	// --------------------------------------------------------------------------------------------
	// Returns:    int - number of chars written (0 means error)
	// Parameters: LPCSTR		szMultiByteString		(IN)	Multi-byte input string
	//					wchar_t*		szUnicodeString		(OUT)	Unicode output string
	//					size_t		nUnicodeBufferSize	(IN)	Size of Unicode output buffer
	//					UINT			nCodePage				(IN)	Code page used to perform conversion
	//																		Default = CP_ACP (Get local code page).
	//
	// Purpose:		Gets a Unicode string from a MultiByte string.
	// Notes:		None.
	// Exceptions:	None.
	//
	static int		GetUnicodeStringFromMultiByteString(IN LPCSTR szMultiByteString, OUT wchar_t* szUnicodeString,
																			IN const size_t nUnicodeBufferSize, IN UINT nCodePage=CP_ACP);

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
	static int		GetRequiredUnicodeLengthFromMultiByteString(IN LPCSTR szMultiByteString, IN UINT nCodePage=CP_ACP OPTIONAL);

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
//	static int		GetNewUnicodeStringFromMultiByteString(IN LPCSTR szMultiByteString, OUT wchar_t*& pszUnicodeString,
//																			 IN UINT nCodePage=CP_ACP OPTIONAL);
	static int		GetNewUnicodeStringFromMultiByteString(IN LPCSTR szMultiByteString, IN OUT CTemplateSmartPtrArray<wchar_t>& spUnicodeString, IN const int nDefaultBufferSize=0,
																			 IN UINT nCodePage=CP_ACP OPTIONAL);

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
	// Notes:		.
	// Exceptions:	None.
	//
	static int			GetMultiByteStringFromUnicodeString(IN const wchar_t * szUnicodeString, OUT char* szMultiByteString,
																			IN const int nMultiByteBufferSize,IN UINT nCodePage=CP_ACP OPTIONAL,
																			IN char cFillerChar=sDEFAULT_UNICODE_FILLER_CHAR OPTIONAL);


	//---------------------------------------------------------------------------------------------------
	//
	//	CStdioFileEx::GetRequiredMultiByteLengthForUnicodeString()
	//
	//---------------------------------------------------------------------------------------------------
	// Returns:    int - no of bytes required
	// Parameters: IN const wchar_t * szUnicodeString,int nCodePage=-1, char cFillerChar='#'
	//
	// Purpose:		Obtains the multi-byte buffer size needed to accommodate a converted Unicode string.
	//	Notes:		We can't assume that the buffer length is simply equal to the number of characters
	//					because that wouldn't accommodate multibyte characters!
	//
	static int			GetRequiredMultiByteLengthForUnicodeString(IN const wchar_t * szUnicodeString,IN UINT nCodePage=CP_ACP OPTIONAL);

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
	static int			GetNewMultiByteStringFromUnicodeString(IN const wchar_t * szUnicodeString,IN OUT CTemplateSmartPtrArray<char>& spMultiByteString, IN const int nDefaultBufferSize=0,IN UINT nCodePage=CP_ACP OPTIONAL,
																			IN char cFillerChar=sDEFAULT_UNICODE_FILLER_CHAR OPTIONAL);


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
	//					Reformatted, commented and adapted by David Pritchard
	//
	//static bool			GetNewUTF8StringFromUnicodeString(IN const wchar_t* szUnicodeString,OUT unsigned char*& pszUTF8String);

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
	static bool IsFileUnicode(const CString& sFilePath);

	static UINT	GetCurrentLocaleCodePage();

protected:
	UINT	ProcessFlags(UINT& nOpenFlags);//, bool& bCheckForUnicodeOnOpen);
#ifdef _UNICODE
	BOOL	ReadUnicodeLine(OUT CString& sOutputLine);
	BOOL	ReadMultiByteLine(OUT CString& sOutputLine);
	void	WriteUnicodeLine(IN LPCTSTR sInputLine);
	void	WriteMultiByteLine(IN LPCTSTR sInputLine);
#else
	BOOL	ReadUnicodeLine(OUT CString& sOutputLine);
	BOOL	ReadMultiByteLine(OUT CString& sOutputLine);
	void	WriteUnicodeLine(IN LPCTSTR sInputLine);
	void	WriteMultiByteLine(IN LPCTSTR sInputLine);
#endif

	bool		m_bIsUnicodeText;
	UINT		m_nFlags;
	int		m_nFileCodePage;
	char		m_cUnicodeFillerChar;
	bool		m_bWriteBOM;
	bool		m_bReadBOM;
	wchar_t	m_arrUnicodeDefaultBuffer[nSTDIOFILEEX_DEFAULT_BUFFER_SIZE];	// default buffer for strings; used to avoid dynamic allocation where possible
	char		m_arrMultibyteDefaultBuffer[nSTDIOFILEEX_DEFAULT_BUFFER_SIZE]; // default buffer for strings; used to avoid dynamic allocation where possible
	bool		m_bCheckFilePos;
};

// Helper class to switch code pages (for multibyte functions) and then guarantee
// the restoration of the original page even in the event of exceptions.
// Note that we are switching the MULTIBYTE code page here, as opposed to the
// LOCALE code page -- yes, there is a difference, but only insofar as the switch
// affects OS functions. The code pages are really the same code pages, so we can
// mix and match, but if we switch the LOCALE cp certain functions will be affected,
// and if we switch the MB cp, other functions will be affected. So it's important to
// decide which functions we're using. I'm using _mbslen (affected by the MB code page), 
// as opposed to _mbstrlen (affected by the locale page). ggets also makes used of 
// _mbschr, also affected by the multibyte page. Be careful with these obscure
// shenanigans if you decide to use other multibyte functions in your code.
/*class CMBCodePageSwitcher
{
public:
	// Constructor
	CMBCodePageSwitcher(IN const int nTargetCodePage)
	{
		m_nOriginalCodePage = -1;

		// If we don't have a specific target code page, do nothing
		if (nTargetCodePage != -1)
		{
			// Get current locale page and store (always? even if 0?)
			m_nOriginalCodePage = _getmbcp();

			// Code page can't be UTF8 or UTF7!
			ASSERT(nTargetCodePage != CP_UTF7 && nTargetCodePage != CP_UTF8);

			// Switch code page for multibyte functions
			VERIFY (_setmbcp(nTargetCodePage) == 0);
		}
	}

	~CMBCodePageSwitcher()
	{
		// Put things back the way they were if they were changed
		if (m_nOriginalCodePage > -1)
		{
			// Try to restore MB code page, but don't check for success
			// because 0 may fail (?)
			VERIFY( _setmbcp(m_nOriginalCodePage) == 0 );
		}
	}

private:
	int m_nOriginalCodePage;
};*/

#endif // !defined(AFX_STDIOFILEEX_H__41AFE3CA_25E0_482F_8B00_C40775BCDB81__INCLUDED_)
