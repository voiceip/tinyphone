#pragma once

#ifndef SERVER_HEADER_FILE_H
#define SERVER_HEADER_FILE_H

#include <crow.h>
#include <iostream>
#include <fstream>
#include <pjsua2.hpp>

class TinyPhoneHTTPLogHandler : public crow::ILogHandler {
private:
	std::fstream log_writer;
public:
	TinyPhoneHTTPLogHandler(std::string log_file) {
		log_writer.open(log_file, std::fstream::out | std::fstream::app);
	};

	~TinyPhoneHTTPLogHandler() {
		log_writer.flush();
		log_writer.close();
	}

	void log(std::string message, crow::LogLevel /*level*/) override {
		log_writer << message ;
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

	struct context {
	};

	void before_handle(crow::request& /*req*/, crow::response& /*res*/, context& /*ctx*/)
	{
		//CROW_LOG_DEBUG << " - MESSAGE: " << message;
	}

	void after_handle(crow::request& /*req*/, crow::response& /*res*/, context& /*ctx*/)
	{
		// no-op
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
 