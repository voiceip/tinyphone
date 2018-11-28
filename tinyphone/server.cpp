#include "stdafx.h"

#include "server.h"
#include "utils.h"
#include "phone.h"
#include "net.h"

#undef snprintf
#include <nlohmann/json.hpp>

using namespace std;
using namespace pj;
using json = nlohmann::json;


#define DEFAULT_HTTP_SERVER_ERROR_REPONSE  {{ "message", "Something went Wrong :(" }}

namespace tp {
	struct response : crow::response {
		response(int code, const nlohmann::json& _body) : crow::response{ code,  _body.dump() } {
			add_header("Access-Control-Allow-Origin", "*");
			add_header("Access-Control-Allow-Headers", "Content-Type");
			add_header("Content-Type", "application/json");
		}
	};
}

using namespace tp;



void TinyPhoneHttpServer::Start() {

	std::cout << "Starting the HTTP server" << std::endl;
	TinyPhone phone;

	crow::App<TinyPhoneMiddleware> app;
	//app.get_middleware<TinyPhoneMiddleware>().setMessage("tinyphone");
	app.loglevel(crow::LogLevel::Info);
	//crow::logger::setHandler(std::make_shared<TinyPhoneHTTPLogHandler>());	
	int http_port = 6060;

	/* Define HTTP Endpoints */
	CROW_ROUTE(app, "/")([]() {
		json response = {
			{ "message", "Hello World" },
		};
		return tp::response(200, response);
	});

	CROW_ROUTE(app, "/login")
		.methods("POST"_method)
		([&phone](const crow::request& req) {
		try {
			auto x = crow::json::load(req.body);
			if (!x) {
				return tp::response(400, {
					{ "message", "Bad Request" },
				});
			}

			string username = x["username"].s();
			string domain = x["domain"].s();
			string password = x["password"].s();
			string account_name = username + "@" + domain;

			// Add account
			pj_thread_auto_register();
			phone.addAccount(username, domain, password);

			CROW_LOG_INFO << "Registered account " << account_name;

			json response = {
				{ "message", "Account added succesfully" },
			};
			return tp::response(200, response);
		}
		catch (std::exception& e) {
			CROW_LOG_ERROR << "Exception catched : " << e.what();
			return tp::response(500, DEFAULT_HTTP_SERVER_ERROR_REPONSE);
		}
	});

	CROW_ROUTE(app, "/dial")
		.methods("POST"_method)
		([&phone](const crow::request& req) {
		auto dial_uri = (char *)req.body.c_str();

		if (!phone.hasAccounts()) {
			return tp::response(400, {
				{ "message", "No Account Registed Yet" },
			});
		}

		CROW_LOG_INFO << "Dial Request to " << req.body;

		try {
			pj_thread_auto_register();

			SIPCall *call = phone.makeCall(dial_uri);
			string account_name = call->getAccount()->getInfo().uri;

			json response = {
				{ "message",  ("Dialed via " + account_name) },
			};
			return tp::response(200, response);
		}
		catch (std::exception& e) {
			CROW_LOG_ERROR << "Exception catched : " << e.what();
			return tp::response(500, DEFAULT_HTTP_SERVER_ERROR_REPONSE);
		}

	});

	CROW_ROUTE(app, "/logout")
		.methods("POST"_method)
		([&phone]() {
		try {
			pj_thread_auto_register();
			CROW_LOG_INFO << "Atempting logout of TinyPhone";
			phone.logout();
			json response = {
				{ "message",  "Logged Out" },
			};
			return tp::response(200, response);
		}
		catch (std::exception& e) {
			CROW_LOG_ERROR << "Exception catched : " << e.what();
			return tp::response(500, DEFAULT_HTTP_SERVER_ERROR_REPONSE);
		}
	});

	CROW_ROUTE(app, "/exit")
		.methods("POST"_method)
		([&app](const crow::request& req) {
		CROW_LOG_INFO << "Shutdown Request from client: " << req.body;
		app.stop();
		json response = {
			{ "message",  "Server Shutdown Triggered" },
		};
		return tp::response(200, response);
	});

	CROW_ROUTE(app, "/hangup_all")
		.methods("POST"_method)
		([&phone]() {
		pj_thread_auto_register();
		phone.hangupAllCalls();
		json response = {
			{ "message",  "Hangup All Calls Triggered" },
		};
		return tp::response(200, response);
	});


	if (is_tcp_port_in_use(http_port)) {
		PrintErr(("HTTP Port " + to_string(http_port) + " already in use!"));
		getEndpoint()->libDestroy();
		exit(1);
	}

	app.port(http_port)
		//.multithreaded()
		.run();

	std::cout << "Server has been shutdown... Will Exit now...." << std::endl;

	//ep.libDestroy();
	getEndpoint()->libDestroy();

}