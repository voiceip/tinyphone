#pragma once

#ifndef SERVER_HEADER_FILE_H
#define SERVER_HEADER_FILE_H
#define CROW_USE_LOCALTIMEZONE 1

#include <crow.h>
#include <iostream>
#include <fstream>
#include <pjsua2.hpp>
#include <chrono>	
#include "metrics.h"
#include "log.h"
#include "phone.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"


class TinyPhoneHTTPLogHandler : public crow::ILogHandler {
private:
	std::shared_ptr<spdlog::logger> logger;
	// std::fstream log_writer;
	// boost::iostreams::stream_buffer<LoggerSink> sb;
public:
	TinyPhoneHTTPLogHandler(std::string log_file) {
		try {
			logger = spdlog::basic_logger_mt("http_logger", log_file);
		} catch (const spdlog::spdlog_ex &ex) {
			std::cout << "Log init failed: " << ex.what() << std::endl;
		}
		//std::cerr.clear();
		//std::cerr.rdbuf(&sb);
	};

	~TinyPhoneHTTPLogHandler() {
		logger->flush();
		//logger->close();
	}

	void log(const std::string message, crow::LogLevel /*level*/) {
		logger->info(message);
#ifdef _DEBUG
		std::cout << message << std::endl ;
#endif
		logger->flush();
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

	void before_handle(crow::request& req, crow::response& res, context& ctx)
	{
		ctx.start = std::chrono::system_clock::now();
		if (req.method == crow::HTTPMethod::Options){
			res.add_header("Access-Control-Allow-Origin", "*");
			res.add_header("Access-Control-Allow-Methods", "OPTIONS, GET, HEAD, POST, PUT, DELETE");
			res.add_header("Access-Control-Request-Headers", "Content-Type");
			res.end();
		}
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
	tp::TinyPhone* tinyPhone;

private:
	bool running;
	crow::App<TinyPhoneMiddleware> app;

public:
	TinyPhoneHttpServer(pj::Endpoint* ep, std::string log_file) {
		endpoint = ep;
		logfile = log_file;
		running = false;
	}

	~TinyPhoneHttpServer() {
		std::cout << "Shutting Down TinyPhone HTTP Service" << std::endl;
	}

	void Configure();
	
	void Start();

	void Stop();
};

#endif
 
