#include "stdafx.h"


#include <ctime>

#include "server.h"
#include "utils.h"
#include "net.h"
#include "consts.h"
#include "config.h"
#include "log.h"

#include <algorithm>

#ifdef _WIN32
#include <iphlpapi.h>
#include "guicon.h"
#include "splash.h"
#include <windows.h>
#include <shellapi.h>
#include "resource.h"

#ifdef _DEBUG
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Debug-Static.lib")
#else
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Release-Static.lib")
#endif

#pragma comment(lib, "IPHLPAPI.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#endif


#define WM_SYSICON  WM_APP
#define MAX_TOOLTIP_LENGTH 96

using namespace std;
using namespace pj;
using namespace tp;

/*variables*/
UINT WM_TASKBAR = 0;
HWND Hwnd;
HMENU Hmenu;
NOTIFYICONDATA notifyIconData;
TCHAR szTIP[MAX_TOOLTIP_LENGTH] = TEXT("Strowger TinyPhone");
char szClassName[] = "TinyPhone";
Endpoint ep;
SPLASH splashScreen;


namespace tp {
	string sipLogFile;
	string httpLogFile;
	tm* launchDate;
	TinyPhoneHttpServer* tpHttpServer;
}

/*procedures  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void InitNotifyIconData();
void InitPJSUAEndpoint(std::string logfile);
void ExitApplication();


void pj_logerror(pj_status_t status, char * message) {
	if (status != PJ_SUCCESS) {
		CROW_LOG_ERROR << "pjsua returned error : " << status;
	}
}

int WINAPI WinMain(HINSTANCE hThisInstance,		
	HINSTANCE hPrevInstance,
	LPSTR lpszArgument,
	int nCmdShow)
{
	MSG messages;
	Hwnd = CreateDialog(
		hThisInstance,
		MAKEINTRESOURCE(IDD_EMPTY_DIALOG),
		NULL,
		(DLGPROC)WindowProcedure
	);

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

#ifdef _DEBUG
	RedirectIOToConsole();
#endif

	/*Initialize the NOTIFYICONDATA structure only once*/
	InitNotifyIconData();

	splashScreen.Init(Hwnd, hThisInstance, IDB_SPLASH);
	splashScreen.Show();

	InitConfig();

	tp::launchDate = now();
	tp::sipLogFile = GetLogFile(SIP_LOG_FILE, "log");
	tp::httpLogFile = GetLogFile(HTTP_LOG_FILE, "log");

	InitPJSUAEndpoint(tp::sipLogFile);
	TinyPhoneHttpServer server(&ep, tp::httpLogFile);
	tp::tpHttpServer = &server;
	

	CROW_LOG_INFO << "System Mac Address: " << getMACAddress();

	//Run the server in non-ui thread.
	std::thread thread_object([&server]() {
		server.Start();
		ExitApplication();
		PostQuitMessage(0);
		exit(0);
	});

	splashScreen.Hide();

	/* Run the message loop. It will run until GetMessage() returns 0 */
	while (GetMessage(&messages, NULL, 0, 0))
	{
		/* Translate virtual-key messages into character messages */
		TranslateMessage(&messages);
		/* Send message to WindowProcedure */
		DispatchMessage(&messages);
	}

	return messages.wParam;
}

void BrowseToFile(LPCTSTR filename)
{
	ITEMIDLIST *pidl = ILCreateFromPath(filename);
	if (pidl) {
		SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
		ILFree(pidl);
	}
}

/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_CLOSE:
		notifyIconData.uFlags = 0;
		Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
		return 0;
		break;
	case WM_DESTROY:
		ExitApplication();
		exit(0);
		break;
		// Our user defined WM_SYSICON message.
	case WM_SYSICON:
	{

	case WM_CONTEXTMENU:
		switch (lParam)
		{
		case WM_LBUTTONUP:
			break;
		case WM_LBUTTONDBLCLK:
			break;
		case WM_RBUTTONDOWN:

			Hmenu = CreatePopupMenu();
			auto local_address = "Local IP: " + local_ip_address();
			AppendMenu(Hmenu, MF_STRING | MF_DISABLED, ID_TRAY_IP, TEXT(local_address.c_str()));
			AppendMenu(Hmenu, MF_SEPARATOR, 0, TEXT(""));
			AppendMenu(Hmenu, MF_STRING, ID_TRAY_LOGDIR, TEXT("View Logs"));
			AppendMenu(Hmenu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit"));
			//DestroyMenu(hMenu);

			// Get current mouse position.
			POINT curPoint;
			GetCursorPos(&curPoint);
			SetForegroundWindow(Hwnd);

			// TrackPopupMenu blocks the app until TrackPopupMenu returns
			UINT clicked = TrackPopupMenu(Hmenu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hwnd, NULL);

			SendMessage(hwnd, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
			switch (clicked)
			{
			case ID_TRAY_EXIT:
				ExitApplication();
				PostQuitMessage(0);
				exit(0);
				break;
			case ID_TRAY_LOGDIR:
			{
				cout << "Open Folder " << GetLogDir() << endl;
				BrowseToFile(tp::sipLogFile.c_str());
			}
				break;
			default:
				break;
			}
		}
	}
	break;

	case WM_SYSCOMMAND:
		/*In WM_SYSCOMMAND messages, the four low-order bits of the wParam parameter
		are used internally by the system. To obtain the correct result when testing the value of wParam,
		an application must combine the value 0xFFF0 with the wParam value by using the bitwise AND operator.*/
		switch (wParam & 0xFFF0)
		{
		case SC_MINIMIZE:
		case SC_CLOSE:
			//minimize();
			return 0;
			break;
		}
		break;

		// intercept the hittest message..
	case WM_NCHITTEST:
	{
		UINT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
		if (uHitTest == HTCLIENT)
			return HTCAPTION;
		else
			return uHitTest;
	}


	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


void InitNotifyIconData()
{
	memset(&notifyIconData, 0, sizeof(NOTIFYICONDATA));

	notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	notifyIconData.hWnd = Hwnd;
	notifyIconData.uID = ID_TRAY_APP_ICON;
	notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notifyIconData.uCallbackMessage = WM_SYSICON; //Set up our invented Windows Message
	notifyIconData.hIcon = (HICON)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));

	try {
		std::string productVersion;
		GetProductVersion(productVersion);
		char buff[MAX_TOOLTIP_LENGTH];
		snprintf(buff, sizeof(buff), "%s %s", szTIP, productVersion.c_str());
		strncpy_s(notifyIconData.szTip, buff, sizeof(buff));
	}
	catch (...) {
		strncpy_s(notifyIconData.szTip, szTIP, sizeof(szTIP));
	}

	Shell_NotifyIcon(NIM_ADD, &notifyIconData);
}

std::vector<std::string> GetLocalDNSServers() {
	FIXED_INFO *pFixedInfo;
	ULONG ulOutBufLen;
	DWORD dwRetVal;
	IP_ADDR_STRING *pIPAddr;
	std::vector <std::string> dnsServers;


	pFixedInfo = (FIXED_INFO *)MALLOC(sizeof(FIXED_INFO));
	if (pFixedInfo == NULL) {
		printf("Error allocating memory needed to call GetNetworkParams\n");
		return dnsServers;
	}
	ulOutBufLen = sizeof(FIXED_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetNetworkParams(pFixedInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pFixedInfo);
		pFixedInfo = (FIXED_INFO *)MALLOC(ulOutBufLen);
		if (pFixedInfo == NULL) {
			printf("Error allocating memory needed to call GetNetworkParams\n");
			return dnsServers;
		}
	}

	if (dwRetVal = GetNetworkParams(pFixedInfo, &ulOutBufLen) == NO_ERROR) {

		printf("Host Name: %s\n", pFixedInfo->HostName);
		
		printf("DNS Servers:\n");
		dnsServers.push_back(pFixedInfo->DnsServerList.IpAddress.String);
		printf("\t%s\n", pFixedInfo->DnsServerList.IpAddress.String);

		pIPAddr = pFixedInfo->DnsServerList.Next;
		while (pIPAddr != NULL) {
			printf("\t%s\n", pIPAddr->IpAddress.String);
			dnsServers.push_back(pIPAddr->IpAddress.String);
			pIPAddr = pIPAddr->Next;
		}
	}
	else {
		printf("GetNetworkParams failed with error: %d\n", dwRetVal);
		return dnsServers;
	}


	if (pFixedInfo != NULL)
		FREE(pFixedInfo);


	return dnsServers;
}

void InitPJSUAEndpoint(std::string logfile) {
	/* Create endpoint instance! */
	try {
		ep.libCreate();

		// Init library
		EpConfig ep_cfg;
		ep_cfg.logConfig.fileFlags = PJ_O_APPEND;
		ep_cfg.logConfig.filename = logfile;
		ep_cfg.logConfig.consoleLevel = ApplicationConfig.pjLogLevel;
		ep_cfg.logConfig.msgLogging = true;
		ep_cfg.logConfig.level = ApplicationConfig.pjLogLevel;
		ep_cfg.logConfig.decor |= PJ_LOG_HAS_CR | PJ_LOG_HAS_DAY_OF_MON |  PJ_LOG_HAS_MONTH |  PJ_LOG_HAS_YEAR ;
		ep_cfg.uaConfig.userAgent = ApplicationConfig.ua();
		ep_cfg.uaConfig.threadCnt = ApplicationConfig.pjThreadCount;
		if (ApplicationConfig.enableSTUN) {
			ep_cfg.uaConfig.stunServer = ApplicationConfig.stunServers;
		}
		ep_cfg.medConfig.threadCnt = ApplicationConfig.pjMediaThreadCount;
		ep_cfg.medConfig.noVad = ApplicationConfig.disableVAD;
		ep_cfg.medConfig.clockRate = ApplicationConfig.clockRate;

		if (ApplicationConfig.enableNoiseCancel) {
			ep_cfg.medConfig.ecTailLen = ApplicationConfig.ecTailLen;
			ep_cfg.medConfig.ecOptions = PJMEDIA_ECHO_DEFAULT | PJMEDIA_ECHO_USE_NOISE_SUPPRESSOR;
		} else {
			ep_cfg.medConfig.ecTailLen = 0;
		}

		ep.libInit(ep_cfg);

		// Transport Setup
		TransportConfig tcfg;
		int port = 5060;

		switch (ApplicationConfig.transport)
		{
		case PJSIP_TRANSPORT_UDP:
			while (is_udp_port_in_use(port)) {
				port++;
			}
			break;
		case PJSIP_TRANSPORT_TCP:
		case PJSIP_TRANSPORT_TLS:
			while (is_tcp_port_in_use(port)) {
				port++;
			}
			break;
		default:
			break;
		}

		std::string productVersion;
		GetProductVersion(productVersion);

		CROW_LOG_INFO << "Running Product Version: " << productVersion;
		CROW_LOG_INFO << "Using Transport Protocol: " << ApplicationConfig.transport;
		CROW_LOG_INFO << "Using Transport Port: " << port;
		CROW_LOG_INFO << "Using UA: " << ApplicationConfig.ua();
		
		tcfg.port = port;
		auto status = ep.transportCreate((pjsip_transport_type_e)ApplicationConfig.transport, tcfg);
		if (status != PJ_SUCCESS) {
			CROW_LOG_INFO << "pjsua.transportCreate returned status : "  << status ;
		}

		// Configure the DNS resolvers to also handle SRV records
		pjsip_endpoint *endpt = pjsua_get_pjsip_endpt();
		std::vector<std::string> dnsServers = GetLocalDNSServers();
		pj_dns_resolver* resolver;
		pj_logerror(pjsip_endpt_create_resolver(endpt, &resolver),"pjsip_endpt_create_resolver");
		
		struct pj_str_t servers[4];
		for (unsigned int i = 0; i < dnsServers.size() ; ++i) {
			pj_cstr(&servers[i], dnsServers.at(i).c_str());
		}

		pj_dns_resolver_set_ns(resolver, dnsServers.size(), servers, NULL);
		pjsip_endpt_set_resolver(endpt, resolver);
		CROW_LOG_INFO << "PJSUA2 Set DNS Resolvers Done... : " << dnsServers.size();

	}
	catch (Error & err) {
		CROW_LOG_ERROR << "Exception: " << err.info();
		exit(1);
	}

	/* Initialization is done, now start pjsua */
	try {
		ep.libStart();
		CROW_LOG_INFO  << "PJSUA2 Started...";
	}
	catch (Error & err) {
		CROW_LOG_ERROR << "Exception: " << err.info() ;
		exit(1);
	}

}


void ExitApplication() {
#ifdef _DEBUG
	CloseConsole();
#endif
	notifyIconData.uFlags = 0;
	Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
	if(tp::tpHttpServer != nullptr)
		tp::tpHttpServer->Stop();
}
