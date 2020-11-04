#include "stdafx.h"
#include "utils.h"
#include <thread>

#ifdef _WIN32
#include <windows.h>
#include <winver.h>
#include <mmsystem.h>
#include <strsafe.h>
#endif

#include <cryptopp/sha3.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>
#include <fstream>

#include <future>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

using namespace std;

#pragma warning( disable : 4995 )

#define APP
void print_thread_name()
{
	thread::id this_id = this_thread::get_id();
	cout << "Thread id: #" << this_id << endl;
}	

#ifndef _WIN32
#define _T(x) x
#endif

namespace tp {


	void ShowWinAlert(std::string title, std::string message) {
		#ifdef _WIN32
		wchar_t *wmsg = new wchar_t[message.length() + 1]; //memory allocation
		mbstowcs(wmsg, message.c_str(), message.length() + 1);
		MessageBoxW(NULL, wmsg, L"Error!", MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL);
		delete[]wmsg;
		#endif
	}

	void DisplayError(std::string message, OPS mode) {
		#ifdef _WIN32
		HANDLE hConsoleErr = GetStdHandle(STD_ERROR_HANDLE);
		SetConsoleTextAttribute(hConsoleErr, FOREGROUND_RED | FOREGROUND_INTENSITY);
		
		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
			(lstrlen((LPCTSTR)lpMsgBuf)
				+ lstrlen((LPCTSTR)message.c_str()) + 40)
			* sizeof(TCHAR));
		StringCchPrintf((LPTSTR)lpDisplayBuf,
			LocalSize(lpDisplayBuf) / sizeof(TCHAR),
			TEXT("Code: %d: %s"), dw, lpMsgBuf);

		fprintf(stderr, "ERROR: %s\n", message.c_str());
		fprintf(stderr, "ERROR Info: %s\n", (LPTSTR)lpDisplayBuf);
		SetConsoleTextAttribute(hConsoleErr, FOREGROUND_WHITE);

		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);

		PlaySound("SystemHand", NULL, SND_ASYNC);
		if (mode == OPS::ASYNC) {
			std::thread t([message]() {
				ShowWinAlert("Error", message);
			});
			t.detach();
			//std::async(std::launch::async, ShowWinAlert,"Error", message);
		}
		else {
			ShowWinAlert("Error", message);
		}
		#endif
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

	bool GetProductVersion(std::string &version)
	{
		#ifdef _WIN32
		// get the filename of the executable containing the version resource
		TCHAR szFilename[MAX_PATH + 1] = { 0 };
		if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0){
			std::cerr << "GetModuleFileName failed with error %d" <<  GetLastError() << endl;
			return false;
		}

		// allocate a block of memory for the version info
		DWORD dummy;
		DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
		if (dwSize == 0){
			std::cerr <<  "GetFileVersionInfoSize failed with error %d" << GetLastError() << endl;
			return false;
		}
		std::vector<BYTE> data(dwSize);

		// load the version info
		if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0])){
			std::cerr << "GetFileVersionInfo failed with error %d" <<  GetLastError() << endl;
			return false;
		}

		UINT                uiVerLen = 0;
		VS_FIXEDFILEINFO*   pFixedInfo = 0;     // pointer to fixed file info structure get the fixed file info (language-independent) 
		if (VerQueryValue(&data[0], TEXT("\\"), (void**)&pFixedInfo, (UINT *)&uiVerLen) == 0) {
			return false;
		}

		CStringA appVersion;
		appVersion.Format("%u.%u.%u.%u",
			HIWORD(pFixedInfo->dwProductVersionMS),LOWORD(pFixedInfo->dwProductVersionMS),
			HIWORD(pFixedInfo->dwProductVersionLS),LOWORD(pFixedInfo->dwProductVersionLS));
		version = appVersion;
		#endif
		return true;
	}

	std::string sha256(std::string data) {
		
		CryptoPP::SHA3_256 hash;
		CryptoPP::byte digest[CryptoPP::SHA3_256::DIGESTSIZE];

		hash.CalculateDigest(digest, (CryptoPP::byte*)data.c_str(), data.length());

		CryptoPP::HexEncoder encoder;
		std::string output;
		encoder.Attach(new CryptoPP::StringSink(output));
		encoder.Put(digest, sizeof(digest));
		encoder.MessageEnd();

		return output;
	}


	boost::filesystem::path GetLogDir() {
		auto tmp_dir = boost::filesystem::temp_directory_path();
		auto tiny_dir = tmp_dir.append("tinyphone");
		if (!boost::filesystem::exists(tiny_dir))
			boost::filesystem::create_directory(tiny_dir);
		return tiny_dir;
	}
	
	boost::filesystem::path GetAppDir(){
		#ifdef _WIN32
		LPWSTR path = NULL;
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);
		if (!SUCCEEDED(hr)) {
			throw std::runtime_error("GetAPPDIr Errored");
		}
		boost::filesystem::path appDir(win32_utf16_to_utf8(path));
		boost::filesystem::path tiny_dir = appDir.append("tinyphone");
		LocalFree(path);
		if (!boost::filesystem::exists(tiny_dir))
			boost::filesystem::create_directory(tiny_dir);
		return tiny_dir;
		#else
		return GetLogDir();
		#endif
	}

	tm* now() {
		time_t rawtime;
		struct tm * timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		return timeinfo;
	}

	std::string LogFileName(std::string filename, std::string ext, tm* date) {
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = date == nullptr ? localtime(&rawtime) :  date;

		strftime(buffer, sizeof(buffer), "%d-%m-%Y-", timeinfo);
		std::string prefix(buffer);

		return prefix + filename + "." + ext;
	}

	std::string GetLogFile(std::string filename, std::string ext, tm* date) {
		boost::filesystem::path tiny_dir = GetLogDir();
		auto logfile = tiny_dir.append(LogFileName(filename, ext,date));
		return logfile.string();
	}

	std::string file_get_contents(std::string  const& path) throw (std::exception) {
		//return (std::stringstream() << std::ifstream(path).rdbuf()).str();
		std::stringstream buffer;
		buffer << std::ifstream(path).rdbuf();
		return buffer.str();
	}

	bool file_exists(std::string fileName)
	{
	    std::ifstream infile(fileName);
	    return infile.good();
	}

	std::vector<char> file_all_bytes(std::string filename){
		ifstream ifs(filename, ios::binary | ios::ate);
		ifstream::pos_type pos = ifs.tellg();
		std::vector<char>  result(pos);
		ifs.seekg(0, ios::beg);
		ifs.read(&result[0], pos);
		return result;
	}

	std::ifstream::pos_type filesize(std::string filename)
	{
		std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
		return in.tellg();
	}

	char* getMACAddress() {
		char *mac_addr = (char*)malloc(18);

		#ifdef _WIN32
		PIP_ADAPTER_INFO AdapterInfo;
		DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);

		AdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
		if (AdapterInfo == NULL) {
			printf("Error allocating memory needed to call GetAdaptersinfo\n");
			free(mac_addr);
			return NULL; // it is safe to call free(NULL)
		}

		// Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
		if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
			free(AdapterInfo);
			AdapterInfo = (IP_ADAPTER_INFO *)malloc(dwBufLen);
			if (AdapterInfo == NULL) {
				printf("Error allocating memory needed to call GetAdaptersinfo\n");
				free(mac_addr);
				return NULL;
			}
		}

		if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
			// Contains pointer to current adapter info
			PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
			do {
				// technically should look at pAdapterInfo->AddressLength
				//   and not assume it is 6.
				if (pAdapterInfo->AddressLength == 6 && pAdapterInfo->Address[0] != '0') {
					sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
						pAdapterInfo->Address[0], pAdapterInfo->Address[1],
						pAdapterInfo->Address[2], pAdapterInfo->Address[3],
						pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
					printf("Address: %s, mac: %s\n", pAdapterInfo->IpAddressList.IpAddress.String, mac_addr);
					// print them all, return the last one.
					// return mac_addr;
				}
				pAdapterInfo = pAdapterInfo->Next;
			} while (pAdapterInfo);
		}
		free(AdapterInfo);
		#endif
		return mac_addr; // caller must free.
	}


	#ifdef _WIN32
	std::string win32_utf16_to_utf8(const wchar_t* wstr) {

		std::string res;
		// If the 6th parameter is 0 then WideCharToMultiByte returns the number of bytes needed to store the result.
		int actualSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
		if (actualSize > 0) {
			//If the converted UTF-8 string could not be in the initial buffer. Allocate one that can hold it.
			std::vector<char> buffer(actualSize);
			actualSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &buffer[0], static_cast<int>(buffer.size()), nullptr, nullptr);
			res = buffer.data();
		}
		if (actualSize == 0) {
			// WideCharToMultiByte return 0 for errors.
			const std::string errorMsg = "UTF16 to UTF8 failed with error code: " + GetLastError();
			throw std::runtime_error(errorMsg.c_str());
		}
		return res;
	}
	#endif
}
