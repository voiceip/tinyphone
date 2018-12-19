// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
	
#pragma once
#include "targetver.h"

#define _DEVMSI_EXPORTS
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <iostream>
#include <windows.h>
#include <strsafe.h>
#include <msiquery.h>
#include <shellapi.h>
#include <string>

#ifndef UNICODE  
typedef std::string String;
#else
typedef std::wstring String;
#endif

void LogResult(
    __in HRESULT hr,
    __in_z __format_string PCSTR fmt, ...
    );
