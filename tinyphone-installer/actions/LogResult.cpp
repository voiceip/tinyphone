#include "stdafx.h"
#include <stdlib.h>
#include <lmerr.h>

// WiX Header Files:
#include <wcautil.h>
#include <strutil.h>


//
// This code modified from MSDN article 256348
// "How to obtain error message descriptions using the FormatMessage API"
// Currently found at http://support.microsoft.com/kb/256348/en-us

#define ERRMSGBUFFERSIZE 256

/**
 * Use FormatMessage() to look an error code and log the error text.
 *
 * @param dwErrorMsgId The error code to be investigated.
 */
void LogError( DWORD dwErrorMsgId )
{
    HLOCAL pBuffer = NULL;   // Buffer to hold the textual error description.
    DWORD ret = 0;    // Temp space to hold a return value.
    HINSTANCE hInst = NULL;  // Instance handle for DLL.
    BOOL doLookup = TRUE;
    DWORD dwMessageId = dwErrorMsgId;
    LPCSTR pMessage = "Error %d";
    DWORD flags= FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS;

    if ( HRESULT_FACILITY(dwErrorMsgId) == FACILITY_MSMQ ) {
        hInst = LoadLibrary( TEXT("MQUTIL.DLL") );
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
        doLookup = ( NULL != hInst );
    } else if ( dwErrorMsgId >= NERR_BASE && dwErrorMsgId <= MAX_NERR ) {
        hInst = LoadLibrary( TEXT("NETMSG.DLL") );
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
        doLookup = ( NULL != hInst );
    } else if ( HRESULT_FACILITY(dwErrorMsgId) == FACILITY_WIN32 ) {
        // A "GetLastError" error, drop the HRESULT_FACILITY
        dwMessageId &= 0x0000FFFF;
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }

    if ( doLookup ) {
            ret = FormatMessageA( 
                flags,
                hInst, // Handle to the DLL.
                dwMessageId, // Message identifier.
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language.
                (LPSTR)&pBuffer, // Buffer that will hold the text string.
                ERRMSGBUFFERSIZE, // Allocate at least this many chars for pBuffer.
                NULL // No insert values.
                );
    }

    if ( 0 < ret && NULL != pBuffer ) {
        pMessage = (LPSTR)pBuffer;
    }

    // Display the string.
    if ( WcaIsInitialized() ) {
        WcaLogError( dwErrorMsgId, pMessage, dwMessageId );
    } else { 
        // Log to stdout/stderr
        fprintf_s( stderr, pMessage, dwMessageId );
        if ( '\n'!= pMessage[ strlen(pMessage) - 1 ] ) {
            fprintf_s( stderr, "\n");
        }
    }

    // Free the buffer.
    LocalFree( pBuffer );
}

void LogResult(
    __in HRESULT hr,
    __in_z __format_string PCSTR fmt, ...
    )
{
    // This code taken from MSDN vsprintf example found currently at
    // http://msdn.microsoft.com/en-us/library/28d5ce15(v=vs.71).aspx
    // ...and then modified... because it doesn't seem to work!
    va_list args;
    int len;
    char * buffer;

    va_start( args, fmt );
#pragma warning( push )
#pragma warning( disable : 4996 )
    len = _vsnprintf( NULL, 0, fmt, args ) + 1;
#pragma warning( pop )
    buffer = (char*)malloc( len * sizeof(char) );

#ifdef _DEBUG
    ::ZeroMemory(buffer, len);
#endif // _DEBUG
    _vsnprintf_s( buffer, len, len-1, fmt, args );

    // (MSDN code complete)

    // Now that the buffer holds the formatted string, send it to
    // the appropriate output.
    if ( WcaIsInitialized() )
    {
        if ( FAILED( hr ) )
        {
            WcaLogError( hr, buffer );
            LogError( hr );
        }
        else
        {
            WcaLog( LOGMSG_STANDARD, buffer );
        }
    }
    else // Log to stdout/stderr
    {
        if ( FAILED( hr ) )
        {
            fprintf_s( stderr, "%s\n", buffer );
            LogError( hr );
        }
        else
        {
            fprintf_s( stdout, "%s\n", buffer );
        }
    }

    free( buffer );
}
