#include "stdafx.h"
#include "utils.h"
#include <thread>
#include <boost/algorithm/string.hpp>

using namespace std;

void print_thread_name()
{
	thread::id this_id = this_thread::get_id();
	cout << "Thread id: #" << this_id << endl;
}	

namespace tp {

	void DisplayError(std::string message) {

		HANDLE hConsoleErr = GetStdHandle(STD_ERROR_HANDLE);
		SetConsoleTextAttribute(hConsoleErr, FOREGROUND_RED);
		fprintf(stderr, "\nERROR: %s\n", message.c_str());
		SetConsoleTextAttribute(hConsoleErr, FOREGROUND_WHITE);

		wchar_t *wmsg = new wchar_t[message.length() + 1]; //memory allocation
		mbstowcs(wmsg, message.c_str(), message.length() + 1);
		MessageBoxW(NULL, wmsg, L"Error!", MB_ICONEXCLAMATION | MB_OK);
		delete[]wmsg;
	}

	bool IsPSTNNnmber(std::string number)
	{
		bool isDigits = true;
		for (size_t i = 0; i < number.size(); i++)
		{
			if ((number.at(i) > '9' || number.at(i) < '0') && number.at(i) != '*' && number.at(i) != '#' && number.at(i) != '.' && number.at(i) != '-' && number.at(i) != '(' && number.at(i) != ')' && number.at(i) != ' ' && number[0] != '+')
			{
				isDigits = false;
				break;
			}
		}
		return isDigits;
	}

	std::string GetDuration(int sec, bool zero)
	{
		if (sec || zero) {
			int h, m, s;
			s = sec;
			h = s / 3600;
			s = s % 3600;
			m = s / 60;
			s = s % 60;
			if (h) {
				return str(boost::format("%d:%02d:%02d") % h % m % s);
			}
			else {
				return str(boost::format("%d:%02d") % m % s);
			}
		}
		return "";
	}

	std::string AddTransportSuffix(std::string &str, pjsip_transport_type_e transport)
	{
		switch (transport) {
		case PJSIP_TRANSPORT_TCP:
			return str.append(_T(";transport=tcp"));
			break;
		case PJSIP_TRANSPORT_TLS:
			return  str.append(_T(";transport=tls"));
			break;
		}
		return str;
	}

	std::string GetSIPURI(std::string str, /* bool isSimple, bool isLocal, */ std::string domain)
	{
		std::string rab = str;
		boost::to_lower(_T(rab));
		int pos = rab.find(_T("sip:"));
		if (pos == -1) {
			str = _T("sip:") + str;
		}
		pos = rab.find(_T("@"));
		if (pos == -1) {
			str = str + _T("@") + domain;
		}
		/*
		if (str.GetAt(str.GetLength() - 1) == '>')
		{
		str = str.Left(str.GetLength() - 1);
		if (!isSimple) {
		if (!isLocal || !accountSettings.accountId) {
		AddTransportSuffix(str);
		}
		}
		str += _T(">");
		}
		else {
		if (!isSimple) {
		if (!isLocal || !accountSettings.accountId) {
		AddTransportSuffix(str);
		}
		}
		str = _T("<") + str + _T(">");
		}*/
		return str;
	}

	void ParseSIPURI(std::string in, tp::SIPUri* out)
	{
		//	tone_gen.toneslot = -1;
		//	tone_gen = NULL;

		// "WEwewew rewewe" <sip:qqweqwe@qwerer.com;rrrr=tttt;qweqwe=rrr?qweqwr=rqwrqwr>
		// sip:qqweqwe@qwerer.com;rrrr=tttt;qweqwe=rrr?qweqwr=rqwrqwr
		if (in.back() == _T('>')) {
			in = in.substr(0, in.size() - 1);
		}
		out->name = _T("");
		out->user = _T("");
		out->domain = _T("");
		out->parameters = _T("");

		int start = in.find(_T("sip:"));
		int end;
		if (start>0)
		{
			out->name = in.substr(1,start - 4);
			if (boost::iequals(out->name, _T("unknown"))){
				out->name = _T("");
			}
		}
		if (start >= 0){
			start += 4;
		} else {
			start = 0;
		}
		end = in.find(_T("@"), start);
		if (end >= 0)
		{
			out->user = in.substr(start, end - start);
			start = end + 1;
		}
		end = in.find(_T(";"), start);
		if (end >= 0) {
			out->domain = in.substr(start, end - start);
			start = end;
			out->parameters = in.substr(start);
		}
		else {
			end = in.find(_T("?"), start);
			if (end >= 0) {
				out->domain = in.substr(start, end - start);
				start = end;
				out->parameters = in.substr(start);
			}
			else {
				out->domain = in.substr(start);
			}
		}
	}
}