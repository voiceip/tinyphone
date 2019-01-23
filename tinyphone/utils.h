#pragma once

#ifndef UTILS_HEADER_FILE_H
#define UTILS_HEADER_FILE_H

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
#include <boost/filesystem.hpp>
#include <ctime>

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

	std::string file_get_contents(std::string  const& path) throw (std::exception);

	std::vector<char> file_all_bytes(std::string filename);
	
	std::ifstream::pos_type filesize(std::string filename);

	bool file_exists(std::string fileName);

	tm* now();

	char* getMACAddress();

}
#endif
 