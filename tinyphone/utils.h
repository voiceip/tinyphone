#pragma once

#ifndef UTILS_HEADER_FILE_H
#define UTILS_HEADER_FILE_H

#ifdef _MSC_VER
#if _MSC_VER < 1500 // VC++ 8.0 and below
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif
#endif

#include <boost/filesystem.hpp>
#include <pj/pool.h>
#include <pj/string.h>
#include <string.h>
#include <pjsua-lib/pjsua.h>
#include <stdio.h>
#include <thread>
#include <vector>
#include <fstream>
#include <sstream>
#include "consts.h"

#include <ctime>
#include <mutex>


#define synchronized(m) for(std::unique_lock<std::recursive_mutex> lk(m); lk; lk.unlock())


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

static std::vector<std::string> splitString(std::string str, char sep = ',') {
	std::vector<std::string> vecString;
	std::string item;

	std::stringstream stringStream(str);
	while (std::getline(stringStream, item, sep)) {
		vecString.push_back(item);
	}
	return vecString;
}

void print_thread_name();

namespace tp {

	struct SIPUri {
		std::string name;
		std::string user;
		std::string domain;
		std::string parameters;
	};

	enum OPS {
		SYNC, ASYNC
	};

	template <typename I>
	I random(I begin, I end)
	{
		const unsigned long n = std::distance(begin, end);
		const unsigned long divisor = (RAND_MAX + 1l) / n;

		unsigned long k;
		do { k = std::rand() / divisor; } while (k >= n);

		std::advance(begin, k);
		return begin;
	}

	void DisplayError(std::string message, OPS mode);

	bool IsPSTNNnmber(std::string number);

	std::string GetDuration(int sec, bool zero);

	std::string AddTransportSuffix(std::string &str, pjsip_transport_type_e transport);

	std::string GetSIPURI(std::string str, std::string domain);

	void ParseSIPURI(std::string in, tp::SIPUri* out);

	bool GetProductVersion(std::string &version);

	std::string sha256(std::string data);

	std::string LogFileName(std::string filename, std::string ext, tm* date = nullptr);

	std::string GetLogFile(std::string filename, std::string ext = "log", tm* date = nullptr);

	boost::filesystem::path GetLogDir();

	boost::filesystem::path GetAppDir();

	std::string file_get_contents(std::string  const& path) throw (std::exception);

	std::vector<char> file_all_bytes(std::string filename);
	
	std::ifstream::pos_type filesize(std::string filename);

	bool file_exists(std::string fileName);

	tm* now();

	#ifdef _WIN32
	std::string win32_utf16_to_utf8(const wchar_t* wstr);
	#endif
}
#endif
