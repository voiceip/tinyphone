/* 
 * Copyright (C) 2011-2018 MicroSIP (http://www.microsip.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

//-----------------------------------------------------------
// When the status of audio endpoint devices change, the
// MMDevice module calls these methods to notify the client.
//-----------------------------------------------------------

#include <Mmdeviceapi.h>
#include <Dbt.h>
#include "global.h"

#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL)  \
				{ (punk)->Release(); (punk) = NULL; }

class CMMNotificationClient : public IMMNotificationClient
{
	LONG _cRef;
	IMMDeviceEnumerator *_pEnumerator;
	bool hasCallback;
	public:
	CMMNotificationClient() :
		  _cRef(1),
		  _pEnumerator(NULL),
		 hasCallback(false)
	  {
		  HRESULT hr = S_OK;
		  CoInitialize(NULL);
		  if (_pEnumerator == NULL)
		  {
			  // Get enumerator for audio endpoint devices.
			  hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
				  NULL, CLSCTX_INPROC_SERVER,
				  __uuidof(IMMDeviceEnumerator),
				  (void**)&_pEnumerator);
			  if (hr != S_OK) {
				  _pEnumerator = NULL;
			  } else {
				  if (_pEnumerator->RegisterEndpointNotificationCallback(this) == S_OK) {
					hasCallback = true;
				  }
			  }
		  }
	  }

	  ~CMMNotificationClient()
	  {
		  if (hasCallback) {
			  _pEnumerator->UnregisterEndpointNotificationCallback(this);
		  }
		  SAFE_RELEASE(_pEnumerator)
		  CoUninitialize();
	  }

	  // IUnknown methods -- AddRef, Release, and QueryInterface

	  ULONG STDMETHODCALLTYPE AddRef()
	  {
		  return InterlockedIncrement(&_cRef);
	  }

	  ULONG STDMETHODCALLTYPE Release()
	  {
		  ULONG ulRef = InterlockedDecrement(&_cRef);
		  if (0 == ulRef)
		  {
			  delete this;
		  }
		  return ulRef;
	  }

	  HRESULT STDMETHODCALLTYPE QueryInterface(
		  REFIID riid, VOID **ppvInterface)
	  {
		  if (IID_IUnknown == riid)
		  {
			  AddRef();
			  *ppvInterface = (IUnknown*)this;
		  }
		  else if (__uuidof(IMMNotificationClient) == riid)
		  {
			  AddRef();
			  *ppvInterface = (IMMNotificationClient*)this;
		  }
		  else
		  {
			  *ppvInterface = NULL;
			  return E_NOINTERFACE;
		  }
		  return S_OK;
	  }

	  // Callbacks
	  
	  HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(
		  EDataFlow flow, ERole role,
		  LPCWSTR pwstrDeviceId)
	  {
		  return S_OK;
	  }

	  HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId)
	  {
		  return S_OK;
	  };

	  HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId)
	  {
		  return S_OK;
	  }

	  HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(
		  LPCWSTR pwstrDeviceId,
		  DWORD dwNewState)
	  {
		  CWnd *pMainWnd = AfxGetApp()->m_pMainWnd;
		  if (pMainWnd) {
			  ::PostMessage(pMainWnd->m_hWnd, WM_DEVICECHANGE, DBT_DEVNODES_CHANGED, 1);
		  }
		  return S_OK;
	  }

	  HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(
		  LPCWSTR pwstrDeviceId,
		  const PROPERTYKEY key)
	  {
		  return S_OK;
	  }
};
