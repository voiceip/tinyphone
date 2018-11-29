#include "stdafx.h"
#include "utils.h"
#include <thread>

using namespace std;

void DisplayError(std::string message) {
	
	HANDLE hConsoleErr = GetStdHandle(STD_ERROR_HANDLE);
	SetConsoleTextAttribute(hConsoleErr, FOREGROUND_RED);
	fprintf(stderr, "\nERROR: %s\n", message.c_str());
	SetConsoleTextAttribute(hConsoleErr, FOREGROUND_WHITE);

	wchar_t *wmsg = new wchar_t[message.length()+1]; //memory allocation
	mbstowcs(wmsg, message.c_str(), message.length()+1);
	MessageBoxW(NULL, wmsg, L"Error!", MB_ICONEXCLAMATION | MB_OK);
	delete[]wmsg;
	
	
}

void print_thread_name()
{
	thread::id this_id = this_thread::get_id();
	cout << "Thread id: #" << this_id;
}	

