#ifndef jumplist_h__
#define jumplist_h__

#include <string>
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>

class JumpList
{
public:
	JumpList(std::wstring AppID);
	~JumpList();
	bool DeleteJumpList();
	void AddTasks();

private:
	HRESULT _CreateShellLink(PCWSTR pszArguments, PCWSTR pszTitle, IShellLinkW **ppsl, int iconindex = -1);
	ICustomDestinationList *pcdl;
};

#endif // jumplist_h__


/*
#ifndef jumplist_h__
#define jumplist_h__

#include <objectarray.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>
#include <knownfolders.h>
#include <shlobj.h>
#include <string>
#include <map>

class JumpList
{
public:
	JumpList(std::wstring AppID);
	~JumpList();

	HRESULT _CreateShellLink(PCWSTR pszArguments, PCWSTR pszTitle, IShellLink **ppsl, int iconindex, bool WA);
	bool _IsItemInArray(std::wstring path, IObjectArray *poaRemoved);
	HRESULT _AddTasksToList();
	HRESULT _AddCategoryToList();
	HRESULT _AddCategoryToList2();
	bool CreateJumpList(std::wstring pluginpath, std::wstring pref, std::wstring fromstart, 
		std::wstring resume, std::wstring openfile, std::wstring bookmarks, std::wstring pltext, bool recent, 
		bool frequent, bool tasks, bool addbm, bool playlist, const std::wstring bms);
	bool DeleteJumpList();
    bool CleanJumpList();

private:
    bool CleanJL(IApplicationDocumentLists *padl, APPDOCLISTTYPE type);

	ICustomDestinationList *pcdl;
	IObjectCollection *poc;
	HRESULT m_hr;
	std::wstring path;
    std::wstring m_AppID;
    std::wstring s1;
    std::wstring s2;
    std::wstring s3;
    std::wstring s4;
    std::wstring s5;
    std::wstring s6;

    const int max_items_jumplist;
};

#endif // jumplist_h__
*/