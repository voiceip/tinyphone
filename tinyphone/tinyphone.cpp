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

#pragma comment(lib, "ws2_32.lib") 
//#pragma comment(lib, "libpjproject-i386-Win32-vc14-Release-Static-NoVideo.lib")
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Debug-Static.lib")



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
		return "Hello world";
	});

	CROW_ROUTE(app, "/login")
		.methods("POST"_method)
		([&phone](const crow::request& req) {
		try {
			auto x = crow::json::load(req.body);
			if (!x)
				return crow::response(400, "Bad Request");

			string username = x["username"].s();
			string domain = x["domain"].s();
			string password = x["password"].s();
			string account_name = username + "@" + domain;

			// Add account
			pj_thread_auto_register();
			phone.addAccount(username, domain, password);

			CROW_LOG_INFO << "Registered account " << account_name;
			return crow::response(200, "Account added succesfully");
		}
		catch (std::exception& e) {
			CROW_LOG_ERROR << "Exception catched : " << e.what();
			return crow::response(500, "Something Went Wrong");
		}
	});

	CROW_ROUTE(app, "/dial")
		.methods("POST"_method)
		([&phone](const crow::request& req) {
		auto dial_uri = (char *)req.body.c_str();

		if (!phone.hasAccounts())
			return crow::response(400, "No Account Registed Yet");

		SIPAccount* account = phone.getPrimaryAccount();
		string account_name = account->getInfo().uri;

		CROW_LOG_INFO << ("Dial Request to " + req.body + " via account " + account_name);

		try {
			pj_thread_auto_register();

			// Make outgoing call
			Call *call = new SIPCall(*account);
			account->calls.push_back(call);
			CallOpParam prm(true);
			prm.opt.audioCount = 1;
			prm.opt.videoCount = 0;
			call->makeCall(dial_uri, prm);

			return crow::response(200, ("Dialed via " + account_name));
		}
		catch (std::exception& e) {
			CROW_LOG_ERROR << "Exception catched : " << e.what();
			return crow::response(500, "Something Went Wrong");
		}

	});

	CROW_ROUTE(app, "/logout")
		.methods("POST"_method)
		([&phone]() {
		try {
			pj_thread_auto_register();
			CROW_LOG_INFO << "Atempting logout of TinyPhone";
			phone.logout();
			return crow::response(200, "Logged Out");
		}
		catch (std::exception& e) {
			CROW_LOG_ERROR << "Exception catched : " << e.what();
			return crow::response(500, "Something Went Wrong");
		}
	});

	CROW_ROUTE(app, "/exit")
		.methods("POST"_method)
		([&app](const crow::request& req) {
		CROW_LOG_INFO << "Shutdown Request from client: " << req.body;
		app.stop();
		return "Server shutdown";
	});

	CROW_ROUTE(app, "/hangup_all")
		.methods("POST"_method)
		([&ep]() {
		pj_thread_auto_register();
		ep.hangupAllCalls();
		return "Hangup Calls";
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