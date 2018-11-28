#include "stdafx.h"
#include "utils.h"
#include <thread>

using namespace std;

void PrintErr(std::string message) {
	HANDLE hConsoleErr = GetStdHandle(STD_ERROR_HANDLE);
	SetConsoleTextAttribute(hConsoleErr, FOREGROUND_RED);
	fprintf(stderr, "%s\n", message.c_str());
	SetConsoleTextAttribute(hConsoleErr, FOREGROUND_WHITE);
}

void print_thread_name()
{
	thread::id this_id = this_thread::get_id();
	cout << "Thread id: #" << this_id;
}	

