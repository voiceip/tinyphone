#pragma once

#ifndef ACCOUNT_HEADER_FILE_H
#define ACCOUNT_HEADER_FILE_H

#include <pjsua2.hpp>
#include <iostream>

using namespace std;
using namespace pj;

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

	SIPAccount* getAccount() {
		return myAcc;
	}

	virtual void onCallState(OnCallStateParam &prm);
	virtual void onCallMediaState(OnCallMediaStateParam &prm);
};

class SIPAccount : public Account
{
public:
	std::vector<SIPCall *> calls;

public:
	SIPAccount()
	{}

	~SIPAccount()
	{
		std::cout << "*** Account is being deleted: No of calls=" << calls.size() << std::endl;
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
	}

	virtual void onIncomingCall(OnIncomingCallParam &iprm)
	{
		SIPCall *call = new SIPCall(*this, iprm.callId);
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

	switch (ci.state) {
	case PJSIP_INV_STATE_DISCONNECTED:
		myAcc->removeCall(this);
		/* Delete the call */
		delete this;
		break;
	case PJSIP_INV_STATE_CONFIRMED:
		break;
	default:
		break;
	}
}


void SIPCall::onCallMediaState(OnCallMediaStateParam &prm)
{
	CallInfo ci = getInfo();
	// Iterate all the call medias
	for (unsigned i = 0; i < ci.media.size(); i++) {
		if (ci.media[i].type == PJMEDIA_TYPE_AUDIO && getMedia(i)) {
			AudioMedia *aud_med = (AudioMedia *)getMedia(i);
			// Connect the call audio media to sound device
			AudDevManager& mgr = Endpoint::instance().audDevManager();
			aud_med->startTransmit(mgr.getPlaybackDevMedia());
			mgr.getCaptureDevMedia().startTransmit(*aud_med);
		}
	}
}
#endif
