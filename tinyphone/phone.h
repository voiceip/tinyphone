#pragma once
#include "account.h"

class TinyPhone
{
	map<pjsua_acc_id, SIPAccount*> accounts;

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