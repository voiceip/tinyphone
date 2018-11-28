// tinyphone.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <crow.h>
#include "server.h"
#include "utils.h"
#include <pjsua2.hpp>
#include <iostream>
#include <string>
#include "phone.h"

using namespace std;
using namespace pj;
using json = nlohmann::json;

#pragma comment(lib, "ws2_32.lib") 
//#pragma comment(lib, "libpjproject-i386-Win32-vc14-Release-Static-NoVideo.lib")
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Debug-Static.lib")
//#define _CRT_SECURE_NO_WARNINGS 1

#define DEFAULT_HTTP_SERVER_ERROR_REPONSE  {{ "message", "Something went Wrong :(" }}


int main(int argc, char *argv[])
{
	TinyPhone phone;
	Endpoint ep;

	crow::App<TinyPhoneMiddleware> app;
	//app.get_middleware<TinyPhoneMiddleware>().setMessage("tinyphone");
	app.loglevel(crow::LogLevel::Info);
	//crow::logger::setHandler(std::make_shared<TinyPhoneHTTPLogHandler>());	
	int http_port = 6060;


	/* Create pjsua instance! */
	try {
		ep.libCreate();

		// Init library
		EpConfig ep_cfg;
		ep_cfg.logConfig.level = 3;//4
		ep.libInit(ep_cfg);

		// Transport
		TransportConfig tcfg;
		int port = 5060;
		while (udp_port_in_use(port)) {
			port++;
		}

		CROW_LOG_INFO << "Using Transport Port: " << port;

		tcfg.port = port;
		ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
	}
	catch (Error & err) {
		std::cout << "Exception: " << err.info() << std::endl;
		exit(1);
	}


	/* audio device selction */
	pjmedia_aud_dev_index dev_idx;
	int dev_count = pjmedia_aud_dev_count();
	printf("Got %d audio devices\n", dev_count);
	for (dev_idx = 0; dev_idx < dev_count; ++dev_idx) {
		pjmedia_aud_dev_info info;
		auto status = pjmedia_aud_dev_get_info(dev_idx, &info);
		printf("%d. %s (in=%d, out=%d)\n", dev_idx, info.name, info.input_count, info.output_count);
	}

	/* Set audio device*/
	//pjmedia_aud_dev_index dev_idx = PJMEDIA_AUD_DEFAULT_CAPTURE_DEV;
	//status = pjmedia_aud_dev_default_param(dev_idx, &param)

	/* Initialization is done, now start pjsua */
	try {
		ep.libStart();
		std::cout << "PJSUA2 Started..." << std::endl;
	}
	catch (Error & err) {
		std::cout << "Exception: " << err.info() << std::endl;
		exit(1);
	}


	/* Define HTTP Endpoints */
	CROW_ROUTE(app, "/")([]() {
		json response = {
			{ "message", "Hello World" },
		};
		return tp::response(200,response);
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

		CROW_LOG_INFO << "Dial Request to " << req.body ;

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
		([&ep]() {
		pj_thread_auto_register();
		ep.hangupAllCalls();
		json response = {
			{ "message",  "Hangup All Calls Triggered" },
		};
		return tp::response(200, response);
	});


	if (tcp_port_in_use(http_port)) {
		PrintErr(("HTTP Port " + to_string(http_port) + " already in use!"));
		ep.libDestroy();
		exit(1);
	}
		
	app.port(http_port)
		//.multithreaded()
		.run();

	std::cout << "Server has been shutdown... Will Exit now...." << std::endl;

	ep.libDestroy();

	return 0;
}