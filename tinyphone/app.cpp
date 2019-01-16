#include "stdafx.h"
#include <windows.h>
#include <shellapi.h>
#include "resource.h"
#include "guicon.h"
#include "server.h"
#include "utils.h"
#include "net.h"
#include "consts.h"
#include "config.h"
#include <ctime>

#ifdef _DEBUG
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Debug-Static.lib")
#else
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Release-Static.lib")
#endif

#define WM_SYSICON  WM_APP

using namespace std;
using namespace pj;
using namespace tp;

/*variables*/
UINT WM_TASKBAR = 0;
HWND Hwnd;
HMENU Hmenu;
NOTIFYICONDATA notifyIconData;
TCHAR szTIP[64] = TEXT("Strowger TinyPhone");
char szClassName[] = "TinyPhone";
Endpoint ep;

/*procedures  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void InitNotifyIconData();
void InitPJSUAEndpoint(std::string logfile);
void ExitApplication();


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

	InitConfig();

	/*Initialize the NOTIFYICONDATA structure only once*/
	InitNotifyIconData();

	string sipLogFile = GetLogFile(SIP_LOG_FILE,"log");
	string httpLogFile = GetLogFile(HTTP_LOG_FILE,"log");

	InitPJSUAEndpoint(sipLogFile);
	TinyPhoneHttpServer server(&ep, httpLogFile);

	//Run the server in non-ui thread.
	std::thread thread_object([&server]() {
		server.Start();
		ExitApplication();
		PostQuitMessage(0);
		exit(0);
	});

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
				auto path = GetLogFile(SIP_LOG_FILE, "log");
				cout << "Open Folder " << GetLogDir() << endl;
				BrowseToFile(path.c_str());
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
	strncpy_s(notifyIconData.szTip, szTIP, sizeof(szTIP));

	Shell_NotifyIcon(NIM_ADD, &notifyIconData);
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

		if (ApplicationConfig.enableNoiseCancel) {
			ep_cfg.medConfig.noVad = true;
			ep_cfg.medConfig.ecTailLen = PJSUA_DEFAULT_EC_TAIL_LEN;
			ep_cfg.medConfig.ecOptions = PJMEDIA_ECHO_DEFAULT | PJMEDIA_ECHO_USE_NOISE_SUPPRESSOR;
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
}