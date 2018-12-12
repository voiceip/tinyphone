#pragma once

#ifndef ACCOUNT_HEADER_FILE_H
#define ACCOUNT_HEADER_FILE_H

#include <pjsua2.hpp>
#include <iostream>
#include <boost/foreach.hpp>
#include "enum.h"
#include "events.h"
#include "call.h"
#include "utils.h"
#include <thread>
#include <future>
#include <chrono>

using namespace std;
using namespace pj;
 
class SIPAccount : public Account
{
	std::string account_name;
	std::promise<int> create_result_promise;
	int create_result_promise_fullfilled = 0;

public:

	EventStream* eventStream;
	std::string domain;
	std::vector<SIPCall *> calls;

	SIPAccount(std::string name, EventStream* es)
	{
		account_name = name;
		eventStream = es;
	}

	~SIPAccount()
	{
		pj_thread_auto_register();
		std::cout << "Account is being deleted:" << account_name << ". No of calls : " << calls.size() << std::endl;;
		try {
			if (pjsua_acc_is_valid(getId()) != 0) {
				AccountInfo ai = getInfo();
				OnRegStateParam prm{
					200,
					pjsip_status_code::PJSIP_SC_OK
				};
				ai.regIsActive = false;
				eventStream->publishEvent(ai, prm);
			}
			else {
				std::cout << "Underlying Account Already Shutdown" << account_name << std::endl;
			}
		}
		catch (...) {
			std::cerr << "Account Shutdown Error" << account_name << std::endl;
		};
		shutdown();
	}

	std::string Name() {
		return account_name;
	}

	/**
	* removes the call from the account list. 
	* Doesn't delete it, you must delete the reference of it.
	*/
	void removeCall(SIPCall *call){
		for (auto it = calls.begin(); it != calls.end(); ++it) {
			if (*it == call) {
				calls.erase(it);
				break;
			}
		}
	}

	virtual void onRegState(OnRegStateParam &prm) 
	{
		AccountInfo ai = getInfo();
		PJ_LOG(3, (__FILENAME__, "RegStateChange %s : %s, Code: %d", account_name.c_str(), (ai.regIsActive ? " Register" : "Unregister"), prm.code));
		eventStream->publishEvent(ai, prm);
		try {
			if (!create_result_promise_fullfilled) {
				create_result_promise.set_value(prm.code);
				create_result_promise_fullfilled++;
			}
			
		}
		catch (std::future_error& e) {
			UNUSED_ARG(e);
		}
	}

	std::future<int> Create(const AccountConfig &cfg, bool make_default = false) throw(Error) {
		create(cfg, make_default);
		return create_result_promise.get_future();
	}

	void HoldAllCalls() {
		BOOST_FOREACH(SIPCall* c, calls) {
			c->HoldCall();
		}
	}

	virtual void onCallEstablished(SIPCall *call) {
		//hold all the other calls
		BOOST_FOREACH(SIPCall* c, calls) {
			if (c != call) {
				c->HoldCall();
			}
		}
	}

	virtual void onIncomingCall(OnIncomingCallParam &iprm)
	{
		SIPCall *call = new SIPCall(*this, iprm.callId);
		CallInfo ci = call->getInfo();
		CallOpParam prm;

		PJ_LOG(3, (__FILENAME__, "Incomming Call: [%s] [%s]", ci.remoteUri.c_str(), ci.stateText.c_str()));

		eventStream->publishEvent(ci, iprm);

		calls.push_back(call);
		prm.statusCode = pjsip_status_code::PJSIP_SC_OK;
		call->answer(prm);
		onCallEstablished(call);
	}

};


#endif
