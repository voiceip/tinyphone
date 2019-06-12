#pragma once

#ifndef SERVER_HEADER_FILE_H
#define SERVER_HEADER_FILE_H

#include <crow.h>
#include <iostream>
#include <fstream>
#include <pjsua2.hpp>
#include <chrono>	
#include "metrics.h"
#include "log.h"

class TinyPhoneHTTPLogHandler : public crow::ILogHandler {
private:
	std::fstream log_writer;
	boost::iostreams::stream_buffer<LoggerSink> sb;
public:
	TinyPhoneHTTPLogHandler(std::string log_file) {
		log_writer.open(log_file, std::fstream::out | std::fstream::app);
		sb.open(LoggerSink(log_writer));
		std::cerr.clear();
		std::cerr.rdbuf(&sb);
	};

	~TinyPhoneHTTPLogHandler() {
		log_writer.flush();
		log_writer.close();
	}

	void log(const std::string &message, crow::LogLevel /*level*/) override {
		log_writer << message ;
#ifdef _DEBUG
		std::cout << message;
#endif
		log_writer.flush();
	}
};

struct TinyPhoneMiddleware
{
	std::string message;

	TinyPhoneMiddleware(){
		message = "";
	}

	void setMessage(std::string newMsg){
		message = newMsg;
	}

	struct	context {
		std::chrono::system_clock::time_point start;
	};

	void before_handle(crow::request& /*req*/, crow::response& /*res*/, context& ctx)
	{
		ctx.start = std::chrono::system_clock::now();
	}

	void after_handle(crow::request& /*req*/, crow::response& res, context& ctx)
	{
		auto now = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - ctx.start).count();
		tp::MetricsClient.increment("http.response." + std::to_string(res.code));
		tp::MetricsClient.timing("http.response.time", duration);
	}
};

class TinyPhoneHttpServer {
public:
	pj::Endpoint* endpoint;
	std::string logfile;
public:
	TinyPhoneHttpServer(pj::Endpoint* ep, std::string log_file) {
		endpoint = ep;
		logfile = log_file;
	}

	~TinyPhoneHttpServer() {
		std::cout << "Shutting Down TinyPhone HTTP Service" << std::endl;
	}

	
	void Start();
};

#endif
 