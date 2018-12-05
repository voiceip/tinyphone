#pragma once

#ifndef UTILS_HEADER_FILE_H
#define UTILS_HEADER_FILE_H

#include "stdafx.h"
#include <pj/pool.h>
#include <pj/string.h>
#include <string.h>
#include <pjsua-lib/pjsua.h>
#include <stdio.h>
#include <thread>
#include <vector>
#include <sstream>
#include "consts.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

void DisplayError(std::string message);

void print_thread_name();

static PJ_IDEF(pj_str_t) pj_str(std::string str)
{
	char* chr = strdup(str.c_str());
	pj_str_t dst;
	dst.ptr = chr;
	dst.slen = str.length();
	return dst;
}


/*
* pj_thread_auto_register(void)
* Provided by Jim Gomes on pjsip forum
* Note, here we use calloc to allocated storage for thread description
* without freeing the descriptors. This leaks 256 bytes per thread
registration.
*/
static PJ_DEF(pj_status_t) pj_thread_auto_register(void)
{

	pj_status_t rc;

	if (!pj_thread_is_registered())
	{
		pj_thread_desc *p_thread_desc;
		pj_thread_t* thread_ptr;
		p_thread_desc = (pj_thread_desc *)calloc(1, sizeof(pj_thread_desc));
		rc = pj_thread_register("auto_thr%p", *p_thread_desc, &thread_ptr);
	}
	else
	{
		rc = PJ_SUCCESS;
	}
	return rc;
}

#define STRING_REMOVE_CHAR(str, ch) str.erase(std::remove(str.begin(), str.end(), ch), str.end())

static std::vector<std::string> splitString(std::string str, char sep = ',') {
	std::vector<std::string> vecString;
	std::string item;

	std::stringstream stringStream(str);
	while (std::getline(stringStream, item, sep)) {
		vecString.push_back(item);
	}

	return vecString;
}

namespace tp {

	static bool IsPSTNNnmber(std::string number)
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

	static std::string GetDuration(int sec, bool zero)
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

	static std::string AddTransportSuffix(std::string &str, pjsip_transport_type_e transport)
	{
		switch (transport){
		case PJSIP_TRANSPORT_TCP:
			return str.append(_T(";transport=tcp"));
			break;
		case PJSIP_TRANSPORT_TLS:
			return  str.append(_T(";transport=tls"));
			break;
		}
		return str;
	}

	static std::string GetSIPURI(std::string str, /* bool isSimple, bool isLocal, */ std::string domain)
	{
		std::string rab = str;
		boost::to_lower(_T(rab));
		int pos = rab.find(_T("sip:"));
		if (pos == -1){
			str = _T("sip:") + str;
		}
		pos = rab.find(_T("@"));
		if (pos == -1) {
			str = str + _T("@") +  domain;
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
}
#endif
 