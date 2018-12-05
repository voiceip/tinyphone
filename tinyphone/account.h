#pragma once

#ifndef ACCOUNT_HEADER_FILE_H
#define ACCOUNT_HEADER_FILE_H

#include <pj/config_site.h>
#include <pjsua2.hpp>
#include <iostream>
#include <boost/foreach.hpp>
#include "enum.h"
#include "events.h"
#include "call.h"
#include "utils.h"

using namespace std;
using namespace pj;
 
class SIPAccount : public Account
{
	std::string account_name;
	std::string domain;

public:

	EventStream* eventStream;
	std::vector<SIPCall *> calls;

	SIPAccount(std::string name, EventStream* es)
	{
		account_name = name;
		eventStream = es;
	}

	~SIPAccount()
	{
		std::cout << "*** Account is being deleted " << account_name << " : No of calls=" << calls.size() << std::endl;
		try {
			if (pjsua_acc_is_valid(getId()) != 0) {
				pj_thread_auto_register();
				AccountInfo ai = getInfo();
				OnRegStateParam prm{
					200,
					pjsip_status_code::PJSIP_SC_OK
				};
				ai.regIsActive = false;
				eventStream->publishEvent(ai, prm);
			}
		}
		catch (...) {
			cout << "Account Shutdown Error " << account_name << endl;
		};
		shutdown();
	}

	std::string Name() {
		return account_name;
	}

	void removeCall(SIPCall *call)
	{
		for (auto it = calls.begin(); it != calls.end(); ++it) {
			if (*it == call) {
				calls.erase(it);
				break;
			}
		}
	}

	std::vector<SIPCall *> getCalls() {
		return calls;		
	}


	virtual void onRegState(OnRegStateParam &prm)
	{
		AccountInfo ai = getInfo();
		std::cout << (ai.regIsActive ? "*** Register: code=" : "*** Unregister: code=")
			<< prm.code << std::endl;
		eventStream->publishEvent(ai, prm);
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

		std::cout << "*** Incoming Call: " << ci.remoteUri << " ["
			<< ci.stateText << "]" << std::endl;

		eventStream->publishEvent(ci, iprm);

		calls.push_back(call);
		prm.statusCode = pjsip_status_code::PJSIP_SC_OK;
		call->answer(prm);
		onCallEstablished(call);
	}

};


#endif
