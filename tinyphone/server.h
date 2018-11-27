#pragma once

#include "crow.h"
#include <sstream>

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
