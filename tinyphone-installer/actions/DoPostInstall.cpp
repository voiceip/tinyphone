#include "stdafx.h"
#include <atlbase.h>
#include <activeds.h>
#include "actions.h"
#include "CheckResult.h"
#include <vector>
#include <lsalookup.h>
#include <Ntsecapi.h>
#include <subauth.h>
#include <sddl.h>
#include <atlstr.h>
#include <iostream> 
#include <sstream>

#define MAX_COMPUTERNAME_LENGTH 256

HRESULT AddDomainUserToLocalGroup(LPCWSTR pwszComputerName,
    LPCWSTR pwszGroupName, LPCWSTR pwszDomainName, LPCWSTR pwszUserToAdd) {

    HRESULT hr = E_FAIL;
    CComPtr<IADsContainer> spComputer;

    // Build the binding string.
    CComBSTR sbstrBindingString;
    sbstrBindingString = "WinNT://";
    sbstrBindingString += pwszComputerName;
    sbstrBindingString += ",computer";

    // Bind to the computer that contains the group.
    hr = ADsOpenObject(sbstrBindingString,
        NULL,
        NULL,
        ADS_SECURE_AUTHENTICATION,
        IID_IADsContainer,
        (void**)&spComputer);

    if (FAILED(hr)){
        return hr;
    }

    // Bind to the group object.
    CComPtr<IDispatch> spDisp;
    hr = spComputer->GetObject(CComBSTR("group"),
        CComBSTR(pwszGroupName),
        &spDisp);
    if (FAILED(hr)){
        return hr;
    }

    CComPtr<IADsGroup> spGroup;
    hr = spDisp->QueryInterface(IID_IADs, (LPVOID*)&spGroup);
    if (FAILED(hr)){
        return hr;
    }

    // Bind to the member to add.
    sbstrBindingString = "WinNT://";
    sbstrBindingString += pwszDomainName;
    sbstrBindingString += "/";
    sbstrBindingString += pwszUserToAdd;

    hr = spGroup->Add(sbstrBindingString);

    if (HRESULT_FROM_WIN32(ERROR_MEMBER_IN_ALIAS) == hr) {
        //This will occur if the member already exists in the group, which is ok for us.
        hr = S_OK;
    }

    return hr;
}


BOOL GetCurrentDomainInfo(PPOLICY_DNS_DOMAIN_INFO * pDomainInfo){
	BOOL status = FALSE;
	LSA_HANDLE hLSA;
	LSA_OBJECT_ATTRIBUTES oaLsa = { 0 };

	if (NT_SUCCESS(LsaOpenPolicy(NULL, &oaLsa, POLICY_VIEW_LOCAL_INFORMATION, &hLSA)))
	{
		status = NT_SUCCESS(LsaQueryInformationPolicy(hLSA, PolicyDnsDomainInformation, (PVOID *)pDomainInfo));
		LsaClose(hLSA);
	}
	return status;
}

std::wstring getComputerName() {
    wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
    DWORD cchBufferSize = sizeof(buffer) / sizeof(buffer[0]);
    if (!GetComputerNameW(buffer, &cchBufferSize))
        throw std::runtime_error("GetComputerName() failed.");
    return std::wstring(&buffer[0]);
}

HRESULT DEVMSI_API DoPostInstall( int argc, LPWSTR* argv )
{
    HRESULT hr = S_OK;
    LogResult(hr, "Executing PostInstall Script...");

    String computerName = getComputerName();
    LogResult(hr, "Computer-Name %ls", computerName.c_str());

	PPOLICY_DNS_DOMAIN_INFO domainInfo;
	if (GetCurrentDomainInfo(&domainInfo)) {
		if (domainInfo->Sid != NULL) {
			String domainName(domainInfo->DnsDomainName.Buffer);
			LogResult(hr, "System Connected to Domain %ls", domainName.c_str());

		}
		else {
			LogResult(hr, "System Not Connected to Domain");
		}
	}
	else {
		LogResult(hr, "Check Domain Error");
	};
    return S_OK;
} 
