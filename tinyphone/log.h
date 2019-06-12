#pragma once

#ifndef LOG_HEADER_FILE_H
#define LOG_HEADER_FILE_H

#include <iostream>
#include <ctime>
#include <boost/iostreams/concepts.hpp> 
#include <boost/iostreams/stream_buffer.hpp>

namespace tp {
	extern std::string sipLogFile;
	extern std::string httpLogFile;
	extern tm* launchDate;
}


class LoggerSink : public boost::iostreams::sink
{
	std::string buff;
	std::fstream& log_writer;

public:
	LoggerSink(std::fstream& _log_writer) : log_writer(_log_writer){}
	std::streamsize write(const char* s, std::streamsize n)
	{
		buff.append(s, n);
		if (s[n - 1] == '\n') {
			std::string out =  "(" + timestamp() + ") [ERROR   ] ";
			out.append(buff);
			log_writer << out;
#ifdef _DEBUG
			std::cout << out;
#endif // _DEBUG
			buff.clear();
		}
		return n;
	}
private:
	static std::string timestamp()
	{
		char date[32];
		time_t t = time(0);

		tm my_tm;

#ifdef _WIN32
		localtime_s(&my_tm, &t);
#else
		localtime_r(&t, &my_tm);
#endif

		size_t sz = strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &my_tm);
		return std::string(date, date + sz);
	}
};


#endif