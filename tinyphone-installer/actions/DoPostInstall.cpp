#include "stdafx.h"
#include "actions.h"
#include "CheckResult.h"

HRESULT DEVMSI_API DoPostInstall( int argc, LPWSTR* argv )
{
    HRESULT hr = S_OK;
	LogResult(hr, "Executed DoPostInstall Magic");
    return hr;
} 
