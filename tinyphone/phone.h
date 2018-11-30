#pragma once

#ifndef PHONE_HEADER_FILE_H
#define PHONE_HEADER_FILE_H

#include "account.h"
#include <algorithm>
#include <boost/foreach.hpp>

#define THIS_FILE	"phone.h"

class TinyPhone
{
	map<pjsua_acc_id, SIPAccount*> accounts;
	pj::Endpoint* endpoint;

public:
	TinyPhone(pj::Endpoint* ep) {
		endpoint = ep;
	}

	~TinyPhone() {
		std::cout << "Shutting Down TinyPhone" << std::endl;
		Logout();
	}

	void SetCodecs() {
		const pj_str_t ID_ALL = { "*", 1 };
		pjsua_codec_set_priority(&ID_ALL, PJMEDIA_CODEC_PRIO_DISABLED);
		EnableCodec("PCMA/8000/1");
		EnableCodec("PCMU/8000/1");
	}

	void EnableCodec(char* codec_name) {
		auto codec = pj_str(codec_name);
		auto status = pjsua_codec_set_priority(&codec, PJMEDIA_CODEC_PRIO_NORMAL);
		if (status == PJ_SUCCESS)
			PJ_LOG(3, (THIS_FILE, "%s activated", codec.ptr));
		else
			PJ_LOG(3, (THIS_FILE, "Failed activating %s, err=%d", codec.ptr, status));
	}

	std::vector<SIPAccount *> Accounts() {
		std::vector<SIPAccount *> accs;
		auto it = accounts.begin();
		while (it != accounts.end())
		{
			accs.push_back(it->second);
			it++;
		}
		return accs;
	}

	bool HasAccounts() {
		return accounts.size() > 0;
	}

	SIPAccount* PrimaryAccount() {
		if (!HasAccounts())
			return NULL;
		else {
			return accounts.begin()->second;
		}
	}

	SIPAccount* AccountById(int pos) {
		if (!HasAccounts())
			return NULL;
		else {
			return accounts.find(pos)->second;
		}
	}

	SIPAccount* AccountByURI(string uri) {
		if (!HasAccounts())
			return NULL;
		else {
			string full_uri = "sip:" + uri;
			SIPAccount* account = NULL;
			auto it = accounts.begin();
			while (it != accounts.end())
			{
				if (it->second->getInfo().uri == full_uri)
				{
					account = it->second;
					break;
				}
				it++;
			}
			return account;
		}
	}

	void Logout() {
		auto it = accounts.begin();
		while (it != accounts.end()) {
			it->second->shutdown();
			it = accounts.erase(it);
		}
	}

	

	bool AddAccount(string username, string domain, string password) {

		string account_name = SIP_ACCOUNT_NAME(username, domain);
		auto exits = AccountByURI(account_name);
		if (exits) {
			return false;
		}
		else {

			AccountConfig acc_cfg;
			acc_cfg.idUri = ("sip:" + account_name);
			acc_cfg.regConfig.registrarUri = ("sip:" + domain);
			acc_cfg.sipConfig.authCreds.push_back(AuthCredInfo("digest", "*", username, 0, password));

			acc_cfg.regConfig.timeoutSec = 180;
			acc_cfg.regConfig.retryIntervalSec = 30;
			acc_cfg.regConfig.firstRetryIntervalSec = 15;
			acc_cfg.videoConfig.autoTransmitOutgoing = PJ_FALSE;
			acc_cfg.videoConfig.autoShowIncoming = PJ_FALSE;

			SIPAccount *acc(new SIPAccount(account_name));
			acc->create(acc_cfg);

			accounts.insert(pair<pjsua_acc_id, SIPAccount*>(acc->getId(), acc));
			return true;
		}
	}

	SIPCall* MakeCall(string uri) {
		SIPAccount* account = PrimaryAccount();
		return MakeCall(uri, account);
	}

	SIPCall* MakeCall(string uri, SIPAccount* account) {
		SIPCall *call = new SIPCall(*account);
		account->calls.push_back(call);
		CallOpParam prm(true);
		prm.opt.audioCount = 1;
		prm.opt.videoCount = 0;
		call->makeCall(uri, prm);
		account->onCallEstablished(call);
		return call;
	}

	std::vector<SIPCall*> Calls() {
		std::vector<SIPCall *> calls;
		auto it = accounts.begin();
		while (it != accounts.end()) {
			auto account_calls = it->second->getCalls();
			calls.insert(std::end(calls), std::begin(account_calls), std::end(account_calls));
			it++;
		}
		return calls;
	}

	SIPCall* CallById(int call_id) {
		BOOST_FOREACH(SIPCall* c, Calls()) {
			if (c->getId() == call_id)
				return c;
		}
		return NULL;
	}


	void HoldOtherCalls(SIPCall* call) {
		auto all_calls = Calls();
		all_calls.erase(std::remove(all_calls.begin(), all_calls.end(), call), all_calls.end());
		BOOST_FOREACH(SIPCall* c, all_calls) {
			c->HoldCall();
		}
	}

	void Hangup(SIPCall* call) {
		CallOpParam prm;
		call->hangup(prm);
	}

	void HangupAllCalls() {
		endpoint->hangupAllCalls();
	}
};



#endif
