#pragma once

#ifndef SERVER_HEADER_FILE_H
#define SERVER_HEADER_FILE_H

#include <crow.h>
#include <iostream>
#include <pjsua2.hpp>

class TinyPhoneHTTPLogHandler : public crow::ILogHandler {
public:
	void log(std::string message, crow::LogLevel /*level*/) override {
		std::cerr << "ExampleLogHandler -> " << message;
	}
};

struct TinyPhoneMiddleware
{
	std::string message;

	TinyPhoneMiddleware()
	{
		message = "foo";
	}

	void setMessage(std::string newMsg)
	{
		message = newMsg;
	}

	struct context
	{
	};

	void before_handle(crow::request& /*req*/, crow::response& /*res*/, context& /*ctx*/)
	{
		CROW_LOG_DEBUG << " - MESSAGE: " << message;
	}

	void after_handle(crow::request& /*req*/, crow::response& /*res*/, context& /*ctx*/)
	{
		// no-op
	}
};

class TinyPhoneHttpServer {
private:
	pj::Endpoint* m_ep;
public:
	TinyPhoneHttpServer(pj::Endpoint* ep) {
		SetEndpoint(ep);
	}

	~TinyPhoneHttpServer() {
		std::cout << "Shutting Down TinyPhone HTTP Service" << std::endl;
	}

	void SetEndpoint(pj::Endpoint* ep) {
		m_ep = ep;
	}

	pj::Endpoint* getEndpoint() {
		return m_ep;
	}



	void Start();
};

#endif
 