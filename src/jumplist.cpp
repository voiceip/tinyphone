#include "jumplist.h"
#include "langpack.h"
#include "resource.h"
#include "define.h"

typedef HRESULT (WINAPI *PSetCurrentProcessExplicitAppUserModelID)(_In_  PCWSTR AppID);

JumpList::JumpList(const std::wstring AppID) :
pcdl(0)
{
	HMODULE hinstDll = LoadLibrary(TEXT("shell32.dll"));
	if (hinstDll) {
		PSetCurrentProcessExplicitAppUserModelID pSetCurrentProcessExplicitAppUserModelID = 
			(PSetCurrentProcessExplicitAppUserModelID)GetProcAddress(hinstDll, "SetCurrentProcessExplicitAppUserModelID");
		if (pSetCurrentProcessExplicitAppUserModelID) {
			pSetCurrentProcessExplicitAppUserModelID(AppID.c_str());
			HRESULT hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
			if (SUCCEEDED(hr)) {
				pcdl->SetAppID(AppID.c_str());
			}
		}
		FreeLibrary(hinstDll);
	}
}

JumpList::~JumpList()
{
	if (pcdl)
	{
		pcdl->Release();
	}
}

bool JumpList::DeleteJumpList()
{
    return (pcdl && pcdl->DeleteList(NULL) == S_OK);
}


// Creates a CLSID_ShellLink to insert into the Tasks section of the Jump List.  This type of Jump
// List item allows the specification of an explicit command line to execute the task.
HRESULT JumpList::_CreateShellLink(PCWSTR pszArguments, PCWSTR pszTitle, IShellLinkW **ppsl, int iconindex)
{
    IShellLinkW *psl;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
    if (SUCCEEDED(hr))
    {
		std::wstring fname;
		fname.resize(MAX_PATH);
		GetModuleFileNameW(0, &fname[0], MAX_PATH);

		fname.resize(wcslen(fname.c_str()));
		std::wstring shortfname;
		shortfname.resize(MAX_PATH);			
		GetShortPathNameW(fname.c_str(), &shortfname[0], MAX_PATH);
		shortfname.resize(wcslen(shortfname.c_str()));
		if (iconindex>=0) {
			psl->SetIconLocation(shortfname.c_str(), -iconindex);
		}
		hr = psl->SetPath(shortfname.c_str());
		if (SUCCEEDED(hr))
        {
            hr = psl->SetArguments(pszArguments);
            if (SUCCEEDED(hr))
            {
                // The title property is required on Jump List items provided as an IShellLink
                // instance.  This value is used as the display name in the Jump List.
                IPropertyStore *pps;
                hr = psl->QueryInterface(IID_PPV_ARGS(&pps));
                if (SUCCEEDED(hr))
                {
                    PROPVARIANT propvar;
                    hr = InitPropVariantFromString(pszTitle, &propvar);
                    if (SUCCEEDED(hr))
                    {
                        hr = pps->SetValue(PKEY_Title, propvar);
                        if (SUCCEEDED(hr))
                        {
                            hr = pps->Commit();
                            if (SUCCEEDED(hr))
                            {
                                hr = psl->QueryInterface(IID_PPV_ARGS(ppsl));
                            }
                        }
                        PropVariantClear(&propvar);
                    }
                    pps->Release();
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        psl->Release();
    }
	return hr;
}

void JumpList::AddTasks()
{
	if (pcdl)
	{
		UINT cMinSlots;
		IObjectArray *poaRemoved;
		HRESULT hr = pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved));

		if (SUCCEEDED(hr))
		{

			IObjectCollection *poc;
			hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));
			if (SUCCEEDED(hr))
			{
				IShellLinkW * psl;
#ifdef _UNICODE
				hr = _CreateShellLink(L"/exit",Translate(TEXT("Exit")), &psl, IDI_EXIT);
#else
				WCHAR pWideString[50];
				MultiByteToWideChar(CP_ACP,0,Translate(TEXT("Exit")),-1,pWideString,50);
				hr = _CreateShellLink(L"/exit", pWideString, &psl, IDI_EXIT);
#endif
				if (SUCCEEDED(hr))
				{
					hr = poc->AddObject(psl);
					psl->Release();
				}
				//--
				if (SUCCEEDED(hr))
				{
					IObjectArray * poa;
					hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
					if (SUCCEEDED(hr))
					{
						// Add the tasks to the Jump List. Tasks always appear in the canonical "Tasks"
						// category that is displayed at the bottom of the Jump List, after all other
						// categories.
						hr = pcdl->AddUserTasks(poa);
						poa->Release();
					}
				}
				poc->Release();
			}
			hr = pcdl->CommitList();
			poaRemoved->Release();
		}
	}
}

/*
#include <objectarray.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>
#include <knownfolders.h>
#include <shlobj.h>
#include <string>
#include <sstream>
#include <map>
#include <strsafe.h>
#include "jumplist.h"
#include "api.h"

JumpList::JumpList(const std::wstring AppID) :
m_AppID(AppID),
max_items_jumplist(100)
{
    m_hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
    pcdl->SetAppID(m_AppID.c_str());
}

JumpList::~JumpList()
{
    pcdl->Release();
}

// Creates a CLSID_ShellLink to insert into the Tasks section of the Jump List.  This type of Jump
// List item allows the specification of an explicit command line to execute the task.
HRESULT JumpList::_CreateShellLink(PCWSTR pszArguments, PCWSTR pszTitle, IShellLink **ppsl, int iconindex, bool WA)
{
    IShellLink *psl;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
    if (SUCCEEDED(hr))
    {
        psl->SetIconLocation(path.c_str(), iconindex);
        if (WA)
        {
            std::wstring fname;
            fname.resize(MAX_PATH);
            GetModuleFileName(0, &fname[0], MAX_PATH);
            fname.resize(wcslen(fname.c_str()));
            std::wstring shortfname;
            shortfname.resize(MAX_PATH);			
            GetShortPathName(fname.c_str(), &shortfname[0], MAX_PATH);
            shortfname.resize(wcslen(shortfname.c_str()));
            hr = psl->SetPath(shortfname.c_str());
        }
        else
            hr = psl->SetPath(L"rundll32.exe");

        if (SUCCEEDED(hr))
        {
            hr = psl->SetArguments(pszArguments);
            if (SUCCEEDED(hr))
            {
                // The title property is required on Jump List items provided as an IShellLink
                // instance.  This value is used as the display name in the Jump List.
                IPropertyStore *pps;
                hr = psl->QueryInterface(IID_PPV_ARGS(&pps));
                if (SUCCEEDED(hr))
                {
                    PROPVARIANT propvar;
                    hr = InitPropVariantFromString(pszTitle, &propvar);
                    if (SUCCEEDED(hr))
                    {
                        hr = pps->SetValue(PKEY_Title, propvar);
                        if (SUCCEEDED(hr))
                        {
                            hr = pps->Commit();
                            if (SUCCEEDED(hr))
                            {
                                hr = psl->QueryInterface(IID_PPV_ARGS(ppsl));
                            }
                        }
                        PropVariantClear(&propvar);
                    }
                    pps->Release();
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        psl->Release();
    }
    return hr;
}

bool JumpList::CreateJumpList(std::wstring pluginpath, std::wstring pref, std::wstring fromstart, 
            std::wstring resume, std::wstring openfile, std::wstring bookmarks, std::wstring pltext, bool recent, 
            bool frequent, bool tasks, bool addbm, bool playlist, const std::wstring bms)
{
    path = pluginpath;
    s1 = pref;
    s2 = fromstart;
    s3 = resume;
    s4 = openfile;
    s5 = bookmarks;
    s6 = pltext;

    UINT cMinSlots;
    IObjectArray *poaRemoved;
    HRESULT hr = pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved));

    CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));

    if (!bms.empty() && addbm && hr == S_OK)
    {
        std::wstringstream ss(bms);
        std::wstring line1, line2;
        bool b = false;
        while (getline(ss, line1))
        {
            if (b)		
            {				
                IShellLink * psl;
                hr = _CreateShellLink(line2.c_str(), line1.c_str(), &psl, 4, true);

                if (!_IsItemInArray(line2, poaRemoved))
                {
                    psl->SetDescription(line2.c_str());
                    poc->AddObject(psl);				
                }
                psl->Release();			
                b = false;
            }
            else
            {
                line2.resize(MAX_PATH);
                if (GetShortPathName(line1.c_str(), &line2[0], MAX_PATH) == 0)
                    line2 = line1;
                else
                    line2.resize(wcslen(line2.c_str()));
                b = true;
            }					
        }
    }

    if (SUCCEEDED(hr))
    {
        if (recent)
            pcdl->AppendKnownCategory(KDC_RECENT);

        if (frequent)
            pcdl->AppendKnownCategory(KDC_FREQUENT);
        
        if (addbm && !bms.empty())
            _AddCategoryToList();	

        if (playlist)
            hr = _AddCategoryToList2();

        if (tasks)
            _AddTasksToList();

        if (SUCCEEDED(hr))
        {
            // Commit the list-building transaction.
            hr = pcdl->CommitList();
        }
    }
    poaRemoved->Release();
    poc->Release();

    return (hr == S_OK);
}

bool JumpList::DeleteJumpList()
{
    return (pcdl->DeleteList(NULL) == S_OK);
}

HRESULT JumpList::_AddTasksToList()
{
    IObjectCollection *poc;
    HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));
    if (SUCCEEDED(hr))
    {
        std::wstring args;
        args = path + L",_pref@0";;

        IShellLink * psl;
        hr = _CreateShellLink(args.c_str(), s1.c_str(), &psl, 0, false);
        if (SUCCEEDED(hr))
        {
            hr = poc->AddObject(psl);
            psl->Release();
        }

        args = path + L",_openfile@0";

        if (SUCCEEDED(hr))
        {
            hr = _CreateShellLink(args.c_str(), s4.c_str(), &psl, 1, false);
            if (SUCCEEDED(hr))
            {
                hr = poc->AddObject(psl);
                psl->Release();
            }
        }

        args = path + L",_resume@0";

        if (SUCCEEDED(hr))
        {
            hr = _CreateShellLink(args.c_str(), s3.c_str(), &psl, 3, false);
            if (SUCCEEDED(hr))
            {
                hr = poc->AddObject(psl);
                psl->Release();
            }
        }

        args = path + L",_fromstart@0";

        if (SUCCEEDED(hr))
        {
            hr = _CreateShellLink(args.c_str(), s2.c_str(), &psl, 2, false);
            if (SUCCEEDED(hr))
            {
                hr = poc->AddObject(psl);
                psl->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            IObjectArray * poa;
            hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
            if (SUCCEEDED(hr))
            {
                // Add the tasks to the Jump List. Tasks always appear in the canonical "Tasks"
                // category that is displayed at the bottom of the Jump List, after all other
                // categories.
                hr = pcdl->AddUserTasks(poa);
                poa->Release();
            }
        }
        poc->Release();
    }
    return hr;
}

// Determines if the provided IShellItem is listed in the array of items that the user has removed
bool JumpList::_IsItemInArray(std::wstring path, IObjectArray *poaRemoved)
{
    bool fRet = false;
    UINT cItems;
    if (SUCCEEDED(poaRemoved->GetCount(&cItems)))
    {
        IShellLink *psiCompare;
        for (UINT i = 0; !fRet && i < cItems; i++)
        {
            if (SUCCEEDED(poaRemoved->GetAt(i, IID_PPV_ARGS(&psiCompare))))
            {
                std::wstring removedpath;
                removedpath.resize(MAX_PATH);
                fRet = (psiCompare->GetArguments(&removedpath[0], MAX_PATH)== S_OK);
                removedpath.resize(wcslen(removedpath.c_str()));
                fRet = !path.compare(removedpath);

                psiCompare->Release();
            }
        }
    }
    return fRet;
}

// Adds a custom category to the Jump List.  Each item that should be in the category is added to
// an ordered collection, and then the category is appended to the Jump List as a whole.
HRESULT JumpList::_AddCategoryToList()
{
    IObjectArray *poa;
    HRESULT hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
    if (SUCCEEDED(hr))
    {
        // Add the category to the Jump List.  If there were more categories, they would appear
        // from top to bottom in the order they were appended.
        hr = pcdl->AppendCategory(s5.c_str(), poa);
        poa->Release();
    }

    return hr;
}

// Adds a custom category to the Jump List.  Each item that should be in the category is added to
// an ordered collection, and then the category is appended to the Jump List as a whole.
HRESULT JumpList::_AddCategoryToList2()
{
    IObjectCollection *poc;
    HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));    

    // enumerate through playlists (need to see if can use api_playlists.h via sdk)
    if(AGAVE_API_PLAYLISTS && AGAVE_API_PLAYLISTS->GetCount()){
        for(size_t i = 0; i < AGAVE_API_PLAYLISTS->GetCount(); i++){
            size_t numItems;
            IShellLink * psl;
            std::wstring title;

            std::wstring tmp;
            tmp.resize(MAX_PATH);

            title = AGAVE_API_PLAYLISTS->GetName(i);

            AGAVE_API_PLAYLISTS->GetInfo(i, api_playlists_itemCount, &numItems, sizeof(numItems));
            StringCchPrintf((LPWSTR)tmp.c_str(),tmp.size(),L" [%d]",numItems);
            title += tmp;
            
            hr = _CreateShellLink(AGAVE_API_PLAYLISTS->GetFilename(i), title.c_str(), &psl, 5, true);
            if (SUCCEEDED(hr))
            {
                psl->SetDescription(AGAVE_API_PLAYLISTS->GetFilename(i));
                hr = poc->AddObject(psl);
                psl->Release();
            }
        }
    }

    IObjectArray *poa;
    hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
    if (SUCCEEDED(hr))
    {
        // Add the category to the Jump List.  If there were more categories, they would appear
        // from top to bottom in the order they were appended.
        hr = pcdl->AppendCategory(s6.c_str(), poa);
        poa->Release();
    }

    return S_OK;
}

bool JumpList::CleanJumpList()
{
    IApplicationDocumentLists *padl;
    HRESULT hr = CoCreateInstance(CLSID_ApplicationDocumentLists, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&padl));

    if (SUCCEEDED(hr))
    {
        CleanJL(padl, ADLT_RECENT);
        CleanJL(padl, ADLT_FREQUENT);
    }

    padl->Release();

    return true;
}

bool JumpList::CleanJL(IApplicationDocumentLists *padl, APPDOCLISTTYPE type)
{
    IObjectArray *poa;
    padl->SetAppID(m_AppID.c_str());
    HRESULT hr = padl->GetList(type, 0, IID_PPV_ARGS(&poa));

    if (SUCCEEDED(hr))
    {
        UINT *count = new UINT;
        hr = poa->GetCount(count);
        if (SUCCEEDED(hr) && (*count) > max_items_jumplist)
        {   
            IApplicationDestinations *pad;
            HRESULT hr = CoCreateInstance(CLSID_ApplicationDestinations, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pad));
            pad->SetAppID(m_AppID.c_str());

            if (SUCCEEDED(hr))
            {
                for (int i = (*count)-1; i > max_items_jumplist; --i)
                {
                    IShellLink *psi;
                    hr = poa->GetAt(i, IID_PPV_ARGS(&psi));

                    if (SUCCEEDED(hr))
                    {
                        try
                        {
                            pad->RemoveDestination(psi);
                        }
                        catch (...)
                        {
                            continue;
                        }                            
                    }
                }
            }                
        }
    }
    else
    {
        wchar_t path[MAX_PATH];
        SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);
        std::wstring filepath(path);
        filepath += L"\\Microsoft\\Windows\\Recent\\AutomaticDestinations\\879d567ffa1f5b9f.automaticDestinations-ms";
        if (DeleteFile(filepath.c_str()) == 0)
        {
            return false;
        }
    }

    return true;
}
*/