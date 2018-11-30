#include "stdafx.h"
#include <windows.h>
#include <shellapi.h>
#include "resource.h"
#include "guicon.h"
#include "server.h"
#include "utils.h"
#include "net.h"
#include "consts.h"

//#pragma comment(lib, "libpjproject-i386-Win32-vc14-Release-Static-NoVideo.lib")
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Debug-Static.lib")

#define WM_SYSICON  WM_APP

using namespace std;
using namespace pj;

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
void InitPJSUAEndpoint();
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
	/*Initialize the NOTIFYICONDATA structure only once*/
	InitNotifyIconData();

	RedirectIOToConsole();

	InitPJSUAEndpoint();
	TinyPhoneHttpServer server(&ep);

	//Run the server in non-ui thread.
	std::thread thread_object([&server]() {
		server.Start();
		ExitApplication();
		PostQuitMessage(0);
		//exit(0);
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


/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{

	case WM_CREATE:
		break;

	case WM_CLOSE:
		return 0;
		break;

	case WM_DESTROY:		
		ExitApplication();
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
			AppendMenu(Hmenu, MF_STRING | MF_DISABLED, ID_TRAY_IP, TEXT("Local IP: 127.0.0.1"));
			AppendMenu(Hmenu, MF_SEPARATOR,0 ,TEXT(""));
			AppendMenu(Hmenu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit"));
			//DestroyMenu(hMenu);

			// Get current mouse position.
			POINT curPoint;
			GetCursorPos(&curPoint);
			SetForegroundWindow(Hwnd);

			// TrackPopupMenu blocks the app until TrackPopupMenu returns
			UINT clicked = TrackPopupMenu(Hmenu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hwnd, NULL);

			SendMessage(hwnd, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
			if (clicked == ID_TRAY_EXIT)
			{
				ExitApplication();
				PostQuitMessage(0);
				exit(0);
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


void InitPJSUAEndpoint() {
	/* Create pjsua instance! */
	try {
		ep.libCreate();

		// Init library
		EpConfig ep_cfg;
		ep_cfg.logConfig.level = 3; //4
		ep_cfg.uaConfig.userAgent = DEFAULT_UA_STRING;

		ep.libInit(ep_cfg);

		// Transport
		TransportConfig tcfg;
		int port = 5060;
		while (is_udp_port_in_use(port)) {
			port++;
		}

		CROW_LOG_INFO << "Using Transport Port: " << port;

		tcfg.port = port;
		ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
	}
	catch (Error & err) {
		std::cout << "Exception: " << err.info() << std::endl;
		exit(1);
	}


	/* audio device selction */
	pjmedia_aud_dev_index dev_idx;
	int dev_count = pjmedia_aud_dev_count();
	printf("Got %d audio devices\n", dev_count);
	for (dev_idx = 0; dev_idx < dev_count; ++dev_idx) {
		pjmedia_aud_dev_info info;
		auto status = pjmedia_aud_dev_get_info(dev_idx, &info);
		printf("%d. %s (in=%d, out=%d)\n", dev_idx, info.name, info.input_count, info.output_count);
	}

	/* Set audio device*/
	//pjmedia_aud_dev_index dev_idx = PJMEDIA_AUD_DEFAULT_CAPTURE_DEV;
	//status = pjmedia_aud_dev_default_param(dev_idx, &param)

	/* Initialization is done, now start pjsua */
	try {
		ep.libStart();
		std::cout << "PJSUA2 Started..." << std::endl;
	}
	catch (Error & err) {
		std::cout << "Exception: " << err.info() << std::endl;
		exit(1);
	}

}

void ExitApplication() {
	// quit the application.
	CloseConsole();
	notifyIconData.uFlags = 0;
	Shell_NotifyIcon(NIM_DELETE, &notifyIconData);

}