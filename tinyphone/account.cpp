#include "stdafx.h"
#include <boost/foreach.hpp>

#include "account.h"
#include "phone.h"
#include "metrics.h"
#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>


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
		try {
			setRegistration(true);
		} catch (pj::Error& e){
			UNUSED_ARG(e);
			//its okay will only happen due to pjsua_acc_set_registration(id, renew) error: Object is busy (PJSIP_EBUSY) (status=171001) [../src/pjsua2/account.cpp:1029]
		}
	}

	void SIPAccount::UnRegister() {
		tp::MetricsClient.increment("account.unregister");
		PJ_LOG(3, (__FILENAME__, "UnRegister %s ", account_name.c_str()));
		try {
			setRegistration(false);
		} catch (pj::Error& e){
			UNUSED_ARG(e);
			//will only happen due to pjsua_acc_set_registration(id, renew) error: Object is busy (PJSIP_EBUSY) (status=171001) [../src/pjsua2/account.cpp:1029]
		}
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

	std::future<int> SIPAccount::Create(const pj::AccountConfig &cfg, bool make_default) {
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
		if (ApplicationConfig.autoDeviceRefresh) {
			phone->refreshDevices();
		}
	}

	void TimedAnswer(void* _call_id){
		int *call_id =  (int*) _call_id;
		PJ_LOG(3, (__FILENAME__, "TimedAnswer Call: [%d]", *call_id));
		pjsua_call_info call_info;
		if (pjsua_call_get_info(*call_id, &call_info) == PJ_SUCCESS) {
			//check if not already answered.
			if(call_info.last_status == 180 || call_info.last_status == 183){
				PJ_LOG(3, (__FILENAME__, "TimedAnswer Call: [%d] Answering Call", *call_id));
				pjsua_call_answer(*call_id,200, NULL, NULL);
			} else {
				PJ_LOG(3, (__FILENAME__, "TimedAnswer Call: [%d] Already Answered", *call_id));
			}
		}
		delete(call_id);
	}
	 
	void SIPAccount::onIncomingCall(OnIncomingCallParam &iprm)
	{
		SIPCall *call = new SIPCall(*this, iprm.callId);
		CallInfo ci = call->getInfo();
		try {

			PJ_LOG(3, (__FILENAME__, "Incoming Call: [%s] [%s]", ci.remoteUri.c_str(), ci.stateText.c_str()));

			eventStream->publishEvent(ci, iprm);
			calls.push_back(call);

			if(ApplicationConfig.autoAnswer && ApplicationConfig.autoAnswerDelay == 0){
				phone->Answer(call);
				onCallEstablished(call);
			} else {
				//play sound
				CallOpParam prm;
				prm.statusCode = pjsip_status_code::PJSIP_SC_RINGING;
				call->answer(prm);
				phone->StartRinging(call);
				
				if(ApplicationConfig.autoAnswer && ApplicationConfig.autoAnswerDelay > 0){
					//TODO: change this to use Endpoint::utilTimerSchedule()
					int *call_id = new int(iprm.callId);
					pjsua_schedule_timer2(&TimedAnswer, (void *)call_id, ApplicationConfig.autoAnswerDelay);
				}
			}
		}
		catch (...) {
			PJ_LOG(3, (__FILENAME__, "ERROR Answering IncomingCall [%s]", ci.remoteUri.c_str()));
			call->Hangup();
			phone->refreshDevices();
		}
	}

}
