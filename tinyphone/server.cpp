#include "stdafx.h"

#include "server.h"
#include "utils.h"
#include "phone.h"
#include "net.h"
#include <unordered_set>
#include <mutex>
#include <future>
#include <thread>
#include <chrono>
#include "channel.h"
#include "json.h"
#include "config.h"
#include "microtar.h"
#include "log.h"
#include <stdio.h>
#include <algorithm>
#include "boost/date_time/posix_time/posix_time.hpp"


using namespace std;
using namespace pj;
using json = nlohmann::json;
using namespace boost::posix_time;


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

	pj_thread_auto_register();
	channel<std::string> updates;
	std::thread ws_publisher_thread;

	CROW_LOG_INFO << "Starting the TinyPhone HTTP API";
	TinyPhone phone(endpoint);
	phone.SetCodecs();

	if (!ApplicationConfig.useDefaultAudioDevice) {
		phone.ConfigureAudioDevices();
	}
	phone.CreateEventStream(&updates);

	crow::App<TinyPhoneMiddleware> app;
	std::mutex mtx;;
	app.get_middleware<TinyPhoneMiddleware>().setMessage("tinyphone");
	app.loglevel(crow::LogLevel::Info);
	crow::logger::setHandler(new TinyPhoneHTTPLogHandler(logfile));
	int http_port = 6060;


	/* Define HTTP Endpoints */
	CROW_ROUTE(app, "/")([]() {
		std::string productVersion;
		GetProductVersion(productVersion);
		json response = {
			{ "message", "Hi!" },
			{ "version", productVersion },
		};
		return tp::response(200, response);
	});

	std::unordered_set<crow::websocket::connection*> subscribers;

	if (tp::ApplicationConfig.enableWSEvents) {

		CROW_ROUTE(app, "/events")
			.websocket()
			.onopen([&](crow::websocket::connection& conn) {
			CROW_LOG_INFO << "new websocket connection";
			std::lock_guard<std::mutex> _(mtx);
			subscribers.insert(&conn);
			json message = {
				{ "message", "welcome" },
				{ "subcription", "created" }
			};
			conn.send_text(message.dump());
		})
			.onclose([&](crow::websocket::connection& conn, const std::string& reason) {
			CROW_LOG_INFO << "websocket connection closed: %s" << reason;
			std::lock_guard<std::mutex> _(mtx);
			subscribers.erase(&conn);
		})
			.onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
			json message = {
				{ "message", "nothing here" },
			};
			conn.send_text(message.dump());
		});
	
		auto _thread = std::thread([&updates, &subscribers]() {
			std::string data;
			while (!updates.is_closed()) {
				if (updates.pop(data, true)) {
					for (auto u : subscribers)
						u->send_text(data);
				}
			}
			CROW_LOG_INFO << "Exiting Channel Relayer Thread ";
			//for (auto u : subscribers)
			//	u->close();
		});
		ws_publisher_thread = std::move(_thread);
	}
	
	CROW_ROUTE(app, "/devices")
		.methods("GET"_method)
		([&phone]() {
		try {
			pj_thread_auto_register();
			AudDevManager& audio_manager = Endpoint::instance().audDevManager();
			audio_manager.refreshDevs();
			AudioDevInfoVector devices = audio_manager.enumDev();
			json response = {
				{ "message",  "Audio Devices" },
				{ "count", devices.size() },
				{ "devices",{} },
			};
			int dev_idx = 0;
			BOOST_FOREACH(AudioDevInfo* info, devices) {
				json dev_info = {
					{ "name",info->name },
					{ "driver",info->driver },
					{ "id", dev_idx },
					{ "inputCount" ,  info->inputCount },
					{ "outputCount" ,  info->outputCount },
				};
				response["devices"].push_back(dev_info);
				dev_idx++;
			}
			return tp::response(200, response);
		}
		catch (...) {
			return tp::response(500, DEFAULT_HTTP_SERVER_ERROR_REPONSE);
		}
	});

	CROW_ROUTE(app, "/login")
		.methods("POST"_method)
		([&phone](const crow::request& req) {
		try {

			json j = json::parse(req.body);
			tp::AccountConfig loginConfig = j.get<tp::AccountConfig>();

			string account_name = SIP_ACCOUNT_NAME(loginConfig.username, loginConfig.domain);

			pj_thread_auto_register();
			CROW_LOG_INFO << "Registering account " << account_name;
			
			auto existing_account = phone.AccountByName(account_name);
			if (existing_account != nullptr) {
				phone.EnableAccount(existing_account);
				return tp::response(208, {
					{ "message", "Account already exists" },
					{ "account_name", account_name },
					{ "id", existing_account->getId() },
				});
			}
			else if (phone.Accounts().size() >= ApplicationConfig.maxAccounts) {
				return tp::response(403, {
					{ "message", "Max Account Allowed Reached." },
				});
			}
			else {
				future<int> result = phone.AddAccount(loginConfig);
				auto sync = req.url_params.get("sync") == nullptr ? false : true;
				if (sync) {
					if (result.wait_for(std::chrono::seconds(30)) == std::future_status::ready) {
						return tp::response(200, {
							{ "message", "Account login" },
							{ "account_name", account_name },
							{ "result", result.get() }
						});
					}
					else {
						return tp::response(408, {
							{ "message", "Account login still in progress" },
							{ "account_name", account_name },
							{ "result", 202 }
						});
					}
				}
				else {
					return tp::response(202, {
						{ "message", "Account login in progress" },
						{ "account_name", account_name },
					});
				};
			}
		}
		catch (json::exception& e) {
			return tp::response(400, {
				{ "message", "Bad Request" },
				{ "reason", e.what() },
			});
		}
		catch (pj::Error& e) {
			CROW_LOG_ERROR << "Exception catched : " << e.reason;
			return tp::response(500, DEFAULT_HTTP_SERVER_ERROR_REPONSE);
		}
		catch (std::domain_error& e) {
			if (ApplicationConfig.deviceErrorAlert) {
				tp::DisplayError(MSG_CONTACT_IT_SUPPORT, tp::OPS::ASYNC);
			}
			std::string response_msg = "System Error: " + string(e.what());
			return tp::response(500, {
				{ "message", response_msg },
				{ "result", 503 }
			});
		}
	});

	CROW_ROUTE(app, "/accounts")
		.methods("GET"_method)
		([&phone]() {
		try {
			json response = {
				{ "message",  "Accounts" },
				{ "accounts", json::array() },
			};
			pj_thread_auto_register();
			BOOST_FOREACH(SIPAccount* account, phone.Accounts()) {
				json account_data = {
					{"id" , account->getId() },
					{"uri" , account->getInfo().uri },
					{"name" , account->Name()},
					{"active" , account->getInfo().regIsActive },
					{"status" , account->getInfo().regStatusText }
				};
				response["accounts"].push_back(account_data);
			}
			return tp::response(200, response);
		}
		catch (...) {
			return tp::response(500, DEFAULT_HTTP_SERVER_ERROR_REPONSE);
		}
	});

	CROW_ROUTE(app, "/accounts/<string>/reregister")
		.methods("POST"_method)
		([&phone](string account_name) {
		try {
			pj_thread_auto_register();
			SIPAccount* acc = phone.AccountByName(account_name);
			if (acc != nullptr) {
				acc->reRegister();
				json response = {
					{ "message",  "Re-Register Triggered" },
					{ "account_name", account_name }
				};
				return tp::response(200, response);
			}
			else {
				return tp::response(400, {
					{ "message", "Account Not Found" },
					{ "account_name" , account_name }
				});
			}

		}
		catch (...) {
			return tp::response(500, DEFAULT_HTTP_SERVER_ERROR_REPONSE);
		}
	});

	CROW_ROUTE(app, "/accounts/<string>/logout")
		.methods("POST"_method)
		([&phone](string account_name) {
		try {
			pj_thread_auto_register();
			SIPAccount* acc = phone.AccountByName(account_name);
			if (acc != nullptr) {
				phone.Logout(acc);
				json response = {
					{ "message",  "Logged Out" },
					{ "account_name", account_name },
					{ "result", 200 }
				};
				return tp::response(200, response);
			}
			else {
				return tp::response(400, {
					{ "message", "Account Not Found" },
					{ "account_name" , account_name },
					{ "result", 400 }
				});
			}
		}
		catch (...) {
			return tp::response(500, DEFAULT_HTTP_SERVER_ERROR_REPONSE);
		}
	});

	CROW_ROUTE(app, "/dial")
		.methods("POST"_method)
		([&phone](const crow::request& req) {

		std::string dial_uri = req.body;

		pj_thread_auto_register();

		SIPAccount* account = phone.PrimaryAccount();
		if (account == nullptr) {
			return tp::response(400, {
				{ "message", "No Account Registed/Active Yet" },
			});
		}

		if (account->calls.size() >= ApplicationConfig.maxCalls) {
			return tp::response(429, {
				{ "message", "Max Concurrent Calls Reached. Please try again later." },
			});
		}

		auto sip_uri = tp::GetSIPURI(dial_uri, account->domain);
		auto sip_dial_uri = (char *)sip_uri.c_str();

		CROW_LOG_INFO << "Dial Request to " << sip_dial_uri;
		try {
			SIPCall *call = phone.MakeCall(sip_dial_uri, account);
			string account_name = call->getAccount()->Name();
			json response = {
				{ "message", "Dialling"},
				{ "call_id", call->getId() },
				{ "sid", call->getInfo().callIdString },
				{ "party", sip_dial_uri },
				{ "account", account_name }
			};
			return tp::response(202, response);
		}
		catch (pj::Error& e) {
			CROW_LOG_ERROR << "Exception catched : " << e.reason;
			return tp::response(500, DEFAULT_HTTP_SERVER_ERROR_REPONSE);
		}

	});

	CROW_ROUTE(app, "/calls")
		.methods("GET"_method)
		([&phone]() {
		try {
			pj_thread_auto_register();
			json response = {
				{ "message",  "Current Calls" },
				{ "count",  phone.Calls().size() },
				{ "calls", json::array() },
			};
			BOOST_FOREACH(SIPCall* call, phone.Calls()) {
				if (call->getId() >= 0) {
					auto ci = call->getInfo();
					tp::SIPUri uri;
					tp::ParseSIPURI(ci.remoteUri, &uri);
					json callinfo = {
						{ "id", ci.id },
						{ "sid", ci.callIdString },
						{ "party", ci.remoteUri },
						{ "callerId", uri.user },
						{ "displayName", uri.name },
						{ "state", ci.stateText },
						{ "duration", ci.totalDuration.sec },
						{ "hold", call->HoldState()._to_string() }
					};
					response["calls"].push_back(callinfo);
				}
			}
			return tp::response(200, response);
		}
		catch (...) {
			return tp::response(500, DEFAULT_HTTP_SERVER_ERROR_REPONSE);
		}
	});

	CROW_ROUTE(app, "/calls/<int>/hold")
		.methods("PUT"_method, "DELETE"_method)
		([&phone](const crow::request& req, int call_id) {

		pj_thread_auto_register();
		SIPCall* call = phone.CallById(call_id);
		if (call == nullptr) {
			return tp::response(400, {
				{ "message", "Call Not Found" },
				{ "call_id" , call_id }
			});
		}
		else {
			json response;
			bool status;
			switch (req.method) {
			case crow::HTTPMethod::Put:
				status = call->HoldCall();
				response = {
					{ "message",  "Hold Triggered" },
					{ "call_id" , call_id },
					{ "status" , status }
				};
				break;
			case crow::HTTPMethod::Delete:
				status = call->UnHoldCall();
				response = {
					{ "message",  "UnHold Triggered" },
					{ "call_id" , call_id },
					{ "status" , status }
				};
				break;
			}
			return tp::response(202, response);
		}
	});

	CROW_ROUTE(app, "/calls/<int>/dtmf/<string>")
		.methods("POST"_method)
		([&phone](int call_id, string dtmf_digits) {
		pj_thread_auto_register();

		try {
			SIPCall* call = phone.CallById(call_id);
			if (call == nullptr) {
				return tp::response(400, {
					{ "message", "Call Not Found" },
					{ "call_id" , call_id }
				});
			}
			else {
				call->dialDtmf(dtmf_digits);
				json response = {
					{ "message",  "DTMF Send" },
					{ "call_id", call_id },
					{ "dtmf", dtmf_digits}
				};
				return tp::response(202, response);
			}
		}
		catch (...) {
			return tp::response(500, DEFAULT_HTTP_SERVER_ERROR_REPONSE);
		}
	});

	CROW_ROUTE(app, "/calls/<int>/hangup")
		.methods("POST"_method)
		([&phone](int call_id) {
		pj_thread_auto_register();

		SIPCall* call = phone.CallById(call_id);
		if (call == nullptr) {
			return tp::response(400, {
				{ "message", "Call Not Found" },
				{"call_id" , call_id}
			});
		}
		else {
			phone.Hangup(call);
			json response = {
				{ "message",  "Hangup Triggered" },
				{ "call_id" , call_id }
			};
			return tp::response(202, response);
		}
	});


	CROW_ROUTE(app, "/hangup_all")
		.methods("POST"_method)
		([&phone]() {
		pj_thread_auto_register();
		phone.HangupAllCalls();
		json response = {
			{ "message",  "Hangup All Calls Triggered" },
		};
		return tp::response(202, response);
	});

	CROW_ROUTE(app, "/logout")
		.methods("POST"_method)
		([&phone]() {
		try {
			pj_thread_auto_register();
			CROW_LOG_INFO << "Atempting logout of TinyPhone";
			json response = {
				{ "message",  "Logged Out" },
				{ "result", 200 },
				{ "accounts", json::array() },
			};
			pj_thread_auto_register();
			BOOST_FOREACH(SIPAccount* account, phone.Accounts()) {
				json account_data = {
					{ "id" , account->getId() },
					{ "name" , account->Name() },
				};
				response["accounts"].push_back(account_data);
			}
			phone.Logout();
			return tp::response(200, response);
		}
		catch (...) {
			return tp::response(500, DEFAULT_HTTP_SERVER_ERROR_REPONSE);
		}
	});


	CROW_ROUTE(app, "/logs")
		.methods("GET"_method)
		([](const crow::request& req) {
		try {
			std::string tmp_file = boost::filesystem::temp_directory_path().string() + "/" + boost::filesystem::unique_path().string() + ".tar";
		
			mtar_t tar; tm* pt_tm;
			mtar_open(&tar, tmp_file.c_str(), "w");

			auto date = req.url_params.get("date");
			if (date == nullptr) {
				pt_tm = tp::launchDate;
				mtar_write_file(&tar, LogFileName(SIP_LOG_FILE, "log", tp::launchDate), tp::sipLogFile);
				mtar_write_file(&tar, LogFileName(HTTP_LOG_FILE, "log", tp::launchDate), tp::httpLogFile);
			} else {
				std::string dt = string(date) + " 00:00:00";
				ptime t(time_from_string(dt));
				pt_tm = &to_tm(t);
				mtar_write_file(&tar, LogFileName(SIP_LOG_FILE, "log", pt_tm), GetLogFile(SIP_LOG_FILE, "log", pt_tm));
				mtar_write_file(&tar, LogFileName(HTTP_LOG_FILE, "log", pt_tm), GetLogFile(HTTP_LOG_FILE, "log", pt_tm));
			}
			mtar_finalize(&tar);
			mtar_close(&tar);
	
			auto tar_bytes = file_all_bytes(tmp_file);
			remove(tmp_file.c_str());

			auto response = crow::response(tar_bytes);
			response.set_header("Content-Type", "application/octet-stream"); 

			std::string ip_addr = local_ip_address();
			std::replace(ip_addr.begin(), ip_addr.end(), '.', '_');
			std::string log_file_name = LogFileName("logs-" + ip_addr, "tar", pt_tm);
	
			response.set_header("Content-Disposition", "attachment; filename=\""+ log_file_name +"\"");
			return response;
		}
		catch (...) {
			return crow::response(500, "Something went wrong!");
		}
	});


	CROW_ROUTE(app, "/exit")
		.methods("POST"_method)
		([&app](const crow::request& req) {
		auto it = req.headers.find(HEADER_SECURITY_CODE);
		json response = {
			{"message",  "Server Shutdown Recieved"},
			{ "result",  401 },
		};
		CROW_LOG_INFO << "Shutdown Request from client: " << req.body;
		if (it != req.headers.end()) {
			auto value = it->second;
			auto hash = sha256(SECURITY_SALT + value);
			if (hash == ApplicationConfig.securityCode) {
				CROW_LOG_INFO << "Shutdown Request from client authenticated";
				response["result"] = 200;
				app.stop();
			}
			else {
				CROW_LOG_INFO << "Shutdown Request security code invalid: " << hash ;
				response["message"] = "Incorrect Security Code";
			}
		}
		return tp::response(200, response);
	});

	if (is_tcp_port_in_use(http_port)) {
		tp::DisplayError("Failed to Bind Port! \nIs another instance of tinyphone running?", OPS::SYNC);
	}
	else {
		app.bindaddr("0.0.0.0")
			.port(http_port)
			.multithreaded()
			.run();
	}

	CROW_LOG_INFO << "Terminating current running call(s) if any";

	pj_thread_auto_register();
	phone.HangupAllCalls();
	endpoint->libDestroy();

	CROW_LOG_INFO << "Server has been shutdown... Will Exit now....";

	updates.close();

	if (tp::ApplicationConfig.enableWSEvents) {
		ws_publisher_thread.join();
	}

	CROW_LOG_INFO << "Shutdown Complete..";

}