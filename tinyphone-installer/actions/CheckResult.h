/**
 * Header file for a convenience function.
 */
#pragma once

/**
 *  Log a message and throw an error on failed HRESULT.
 *
 *  @param hr HRESULT to be checked for failure
 *  @param message Message to be sent to the log on failure
 */
inline void CheckResult( HRESULT hr, const char* message )
{
    if ( FAILED(hr) )
    {
        LogResult( hr, message );
        throw( (hr) );
    }
} // CheckResult( HRESULT hr, const char* message )
