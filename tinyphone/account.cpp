#include "stdafx.h"
#include "account.h"
#include "phone.h"

using namespace std;
using namespace pj;

namespace tp {

	std::string SIPAccount::Name() {
		return account_name;
	}

	void SIPAccount::removeCall(SIPCall *call){
		for (auto it = calls.begin(); it != calls.end(); ++it) {
			if (*it == call) {
				calls.erase(it);
				break;
			}
		}
	}

	void SIPAccount::reRegister() {
		PJ_LOG(3, (__FILENAME__, "ReReigstering %s ", account_name.c_str()));
		setRegistration(true);
	}

	void SIPAccount::UnRegister() {
		tp::MetricsClient.increment("account.unregister");
		PJ_LOG(3, (__FILENAME__, "UnRegister %s ", account_name.c_str()));
		setRegistration(false);
	}

	void SIPAccount::onRegState(OnRegStateParam &prm) 
	{
		AccountInfo ai = getInfo();
		PJ_LOG(3, (__FILENAME__, "RegStateChange %s : %s, Code: %d", account_name.c_str(), (ai.regIsActive ? " Register" : "Unregister"), prm.code));
		try {
			eventStream->publishEvent(ai, prm);
			if (!create_result_promise_fullfilled) {
				create_result_promise.set_value(prm.code);
				create_result_promise_fullfilled++;
			}
		}
		catch (std::future_error& e) {
			UNUSED_ARG(e);
		} catch (...){
			PJ_LOG(3, (__FILENAME__, "ERROR: onRegState Unknown Error %s ", account_name.c_str()));
		}
	}

	std::future<int> SIPAccount::Create(const pj::AccountConfig &cfg, bool make_default) throw(Error) {
		create(cfg, make_default);
		return create_result_promise.get_future();
	}

	void SIPAccount::HoldAllCalls() {
		BOOST_FOREACH(SIPCall* c, calls) {
			c->HoldCall();
		}
	}

	void SIPAccount::onCallEstablished(SIPCall *call) {
		//hold all the other calls
		BOOST_FOREACH(SIPCall* c, calls) {
			if (c != call) {
				PJ_LOG(3, (__FILENAME__, "OnCallEstablished Auto Hold Call: [%d]", c->getId()));
				c->HoldCall();
			}
		}
	}

	void SIPAccount::onCallEnd(SIPCall *call) {
		if (ApplicationConfig.autoUnHold) {
			BOOST_FOREACH(SIPCall* c, calls) {
				if (c != call && c->HoldState()._to_integral() != (+HoldStatus::NOT_IN_HOLD)._to_integral() ) {
					PJ_LOG(3, (__FILENAME__, "OnCallEnd Auto UnHold Call: [%d]", c->getId()));
					c->UnHoldCall();
					break;
				}
			}
		}
		phone->refreshDevices();
	}

	void SIPAccount::onIncomingCall(OnIncomingCallParam &iprm)
	{
		SIPCall *call = new SIPCall(*this, iprm.callId);
		CallInfo ci = call->getInfo();
		try {
			CallOpParam prm;

			PJ_LOG(3, (__FILENAME__, "Incoming Call: [%s] [%s]", ci.remoteUri.c_str(), ci.stateText.c_str()));

			eventStream->publishEvent(ci, iprm);

			calls.push_back(call);
			prm.statusCode = pjsip_status_code::PJSIP_SC_OK;
			call->answer(prm);
			onCallEstablished(call);
		}
		catch (...) {
			PJ_LOG(3, (__FILENAME__, "ERROR Answering IncomingCall [%s]", ci.remoteUri.c_str()));
			call->Hangup();
			phone->refreshDevices();
		}
	}

}
