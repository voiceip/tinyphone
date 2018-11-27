// tinyphone.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <pjsua-lib/pjsua.h>
#include <crow.h>
#include "server.h"
#include "utils.h"
#include <pjsua2.hpp>
#include <iostream>

using namespace std;
using namespace pj;

#pragma comment(lib, "ws2_32.lib") 
//#pragma comment(lib, "libpjproject-i386-Win32-vc14-Release-Static-NoVideo.lib")
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Debug-Static.lib")

#define THIS_FILE	"tinyphone"

class SIPAccount;

class SIPCall : public Call
{
private:
	SIPAccount *myAcc;

public:
	SIPCall(Account &acc, int call_id = PJSUA_INVALID_ID)
		: Call(acc, call_id)
	{
		myAcc = (SIPAccount *)&acc;
	}

	virtual void onCallState(OnCallStateParam &prm);
};

class SIPAccount : public Account
{
public:
	std::vector<Call *> calls;

public:
	SIPAccount()
	{}

	~SIPAccount()
	{
		std::cout << "*** Account is being deleted: No of calls=" << calls.size() << std::endl;
	}

	void removeCall(Call *call)
	{
		for (std::vector<Call *>::iterator it = calls.begin(); it != calls.end(); ++it) {
			if (*it == call) {
				calls.erase(it);
				break;
			}
		}
	}

	virtual void onRegState(OnRegStateParam &prm)
	{
		AccountInfo ai = getInfo();
		std::cout << (ai.regIsActive ? "*** Register: code=" : "*** Unregister: code=")
			<< prm.code << std::endl;
	}

	virtual void onIncomingCall(OnIncomingCallParam &iprm)
	{
		Call *call = new SIPCall(*this, iprm.callId);
		CallInfo ci = call->getInfo();
		CallOpParam prm;

		std::cout << "*** Incoming Call: " << ci.remoteUri << " ["
			<< ci.stateText << "]" << std::endl;

		calls.push_back(call);
		prm.statusCode = (pjsip_status_code)200;
		call->answer(prm);
	}
};

void SIPCall::onCallState(OnCallStateParam &prm)
{
	PJ_UNUSED_ARG(prm);

	CallInfo ci = getInfo();
	std::cout << "*** Call: " << ci.remoteUri << " [" << ci.stateText
		<< "]" << std::endl;

	if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
		myAcc->removeCall(this);
		/* Delete the call */
		delete this;
	}
}

class TinyPhone
{
	map<pjsua_acc_id, SIPAccount*> accounts;
	// std::vector<SIPAccount *> accounts;

public:
	TinyPhone() {
	}

	~TinyPhone() {
		std::cout << "Shutting Down TinyPhone" << std::endl;
		logout();
	}

	bool hasAccounts() {
		return accounts.size() > 0;
	}

	SIPAccount* getPrimaryAccount() {
		if (!hasAccounts())
			return NULL;
		else {
			return accounts.begin()->second;
		}
	}

	void logout() {
		auto it = accounts.begin();
		while (it != accounts.end()) {
			it->second->shutdown();
			it++;
		}
	}

	void addAccount(string username, string domain, string password) {

		string account_name = username + "@" + domain;

		AccountConfig acc_cfg;
		acc_cfg.idUri = ("sip:" + account_name);
		acc_cfg.regConfig.registrarUri = ("sip:" + domain);
		acc_cfg.sipConfig.authCreds.push_back(AuthCredInfo("digest", "*", username, 0, password));

		acc_cfg.regConfig.timeoutSec = 180;
		acc_cfg.regConfig.retryIntervalSec = 30;
		acc_cfg.regConfig.firstRetryIntervalSec = 15;
		acc_cfg.videoConfig.autoTransmitOutgoing = PJ_FALSE;
		acc_cfg.videoConfig.autoShowIncoming = PJ_FALSE;

		SIPAccount *acc(new SIPAccount);
		acc->create(acc_cfg);

		accounts.insert(pair<pjsua_acc_id, SIPAccount*>(acc->getId(), acc));
	}

	void makeCall(string destination) {

	}

};


int main(int argc, char *argv[])
{
	TinyPhone phone;
	Endpoint ep;

	crow::App<TinyPhoneMiddleware> app;
	//app.get_middleware<TinyPhoneMiddleware>().setMessage("tinyphone");

	/* Create pjsua instance! */
	try {
		ep.libCreate();

		// Init library
		EpConfig ep_cfg;
		ep_cfg.logConfig.level = 3;//4
		ep.libInit(ep_cfg);

		// Transport
		TransportConfig tcfg;
		tcfg.port = 5060;
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
		std::cout << "*** PJSUA2 STARTED ***" << std::endl;
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
			return crow::response(500, "Dial Failed, Something Went Wrong");
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

	app.loglevel(crow::LogLevel::Info);
	//crow::logger::setHandler(std::make_shared<TinyPhoneHTTPLogHandler>());	

	app.port(6060)
		//.multithreaded()
		.run();

	std::cout << "Server has been shutdown... Will Exit now...." << std::endl;

	ep.libDestroy();

	return 0;
}