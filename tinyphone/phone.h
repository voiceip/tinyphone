#pragma once

#ifndef PHONE_HEADER_FILE_H
#define PHONE_HEADER_FILE_H

#include "account.h"
#include "channel.h"
#include <algorithm>
#include <string>
#include <stdexcept>
#include "events.h"
#include "utils.h"
#include "config.h"
#include "json.h"

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

namespace tp {

	struct AccountConfig {
		string username;
		string domain;
		string password;
		string proxy;
	};

	void from_json(const nlohmann::json& j, AccountConfig& p);

	class TinyPhone
	{
		std::vector<SIPAccount*> accounts;
		pj::Endpoint* endpoint;
		EventStream* eventStream;

	private:
		std::string addTransportSuffix(std::string &str) {
			tp::AddTransportSuffix(str, ApplicationConfig.transport);
			return str;
		}

	public:
		int input_audio_dev = 0, output_audio_dev = 0;

		TinyPhone(pj::Endpoint* ep) {
			endpoint = ep;
		}

		~TinyPhone() {
			pj_thread_auto_register();
			std::cout << "Shutting Down TinyPhone" << std::endl;
			if (endpoint->libGetState() == PJSUA_STATE_RUNNING) {
				HangupAllCalls();
				Logout();
			}
			delete eventStream;
			std::cout << "Shutting Down TinyPhone Complete" << std::endl;
		}

		void SetCodecs() {

			pjsua_codec_info codec[32];
			ZeroMemory(codec, sizeof(codec));
			unsigned uCount = 32;
			if (pjsua_enum_codecs(codec, &uCount) == PJ_SUCCESS) {
				printf("List of supported codecs:\n");
				for (unsigned i = 0; i<uCount; ++i) {
					printf("  %d\t%.*s\n", codec[i].priority,
						(int)codec[i].codec_id.slen, codec[i].codec_id.ptr);
				}
			}


			const pj_str_t ID_ALL = { "*", 1 };
			pjsua_codec_set_priority(&ID_ALL, PJMEDIA_CODEC_PRIO_DISABLED);
			int priority = 0;
			BOOST_FOREACH(std::string codec, ApplicationConfig.audioCodecs) {
				EnableCodec(codec, PJMEDIA_CODEC_PRIO_NORMAL);
			}
		}

		void CreateEventStream(channel<std::string>* ch) {
			eventStream = new EventStream(ch);
		}

		void EnableCodec(std::string codec_name, pj_uint8_t priority) {
			auto codec = pj_str(codec_name);
			auto status = pjsua_codec_set_priority(&codec, priority);
			if (status == PJ_SUCCESS)
				PJ_LOG(3, (__FILENAME__, "%s activated, priority %d", codec.ptr, priority));
			else
				PJ_LOG(3, (__FILENAME__, "Failed activating %s, err=%d", codec.ptr, status));
			free(codec.ptr);
		}

		bool TestAudioDevice() {
			try {
				AudDevManager& audio_manager = Endpoint::instance().audDevManager();
				audio_manager.refreshDevs();
				AudioMedia& cap_med = audio_manager.getCaptureDevMedia();
				AudioMedia& play_med = audio_manager.getPlaybackDevMedia();
				cap_med.startTransmit(play_med);
				pj_thread_sleep(50);
				cap_med.stopTransmit(play_med);
				return true;
			}
			catch (Error& err) {
				PJ_LOG(1, (__FILENAME__, "TestAudioDevice Error %s", err.reason));
				return false;
			}
		}

		void ConfigureAudioDevices();

		void refreshDevices();

		std::vector<SIPAccount *> Accounts() {
			return accounts;
		}

		bool HasAccounts() {
			return accounts.size() > 0;
		}

		SIPAccount* PrimaryAccount() {
			if (!HasAccounts())
				return nullptr;
			else {
				BOOST_FOREACH(SIPAccount* acc, accounts) {
					if (acc->getInfo().regStatus == pjsip_status_code::PJSIP_SC_OK)
						return acc;
				}
				return nullptr;
			}
		}

		SIPAccount* AccountByName(string name) {
			if (!HasAccounts())
				return nullptr;
			else {
				SIPAccount* account = nullptr;
				BOOST_FOREACH(SIPAccount* acc, accounts) {
					if (acc->Name() == name) {
						account = acc;
						break;
					}
				}
				return account;
			}
		}

		void Logout(SIPAccount* acc) {
			auto it = accounts.begin();
			while (it != accounts.end()) {
				if (*it == acc) {
					delete (*it);
					it = accounts.erase(it);
					break;
				}
			}
		}

		void Logout() {
			auto it = accounts.begin();
			while (it != accounts.end()) {
				delete (*it);
				it = accounts.erase(it);
			}
		}

		void EnableAccount(SIPAccount* account) throw (std::exception) {
			if (ApplicationConfig.testAudioDevice && !TestAudioDevice()) {
				throw std::domain_error(MSG_DEVICE_ERROR);
			}
			account->reRegister();
		}

		std::future<int> AddAccount(AccountConfig& config) throw (std::exception) {
			string account_name = SIP_ACCOUNT_NAME(config.username, config.domain);
			auto exits = AccountByName(account_name);
			if (exits != nullptr) {
				throw std::invalid_argument("Account already exists");
			}
			else {
				if (ApplicationConfig.testAudioDevice && !TestAudioDevice()) {
					throw std::domain_error(MSG_DEVICE_ERROR);
				}

				pj::AccountConfig acc_cfg;
				acc_cfg.idUri = ("sip:" + account_name);
				acc_cfg.regConfig.registrarUri = ("sip:" + config.domain);
				
				addTransportSuffix(acc_cfg.regConfig.registrarUri);
				acc_cfg.sipConfig.authCreds.push_back(AuthCredInfo("digest", "*", config.username, 0, config.password));
				
				if (config.proxy.size() > 0) {
					acc_cfg.sipConfig.proxies = { config.proxy };
				}

				acc_cfg.regConfig.timeoutSec = ApplicationConfig.timeoutSec;
				acc_cfg.regConfig.delayBeforeRefreshSec = ApplicationConfig.refreshIntervalSec;
				acc_cfg.regConfig.retryIntervalSec = ApplicationConfig.retryIntervalSec;
				acc_cfg.regConfig.firstRetryIntervalSec = ApplicationConfig.firstRetryIntervalSec;
				acc_cfg.regConfig.dropCallsOnFail = ApplicationConfig.dropCallsOnFail;

				acc_cfg.videoConfig.autoTransmitOutgoing = PJ_FALSE;
				acc_cfg.videoConfig.autoShowIncoming = PJ_FALSE;

				SIPAccount *acc(new SIPAccount(this, account_name, eventStream));
				acc->domain = config.domain;
				auto res = acc->Create(acc_cfg);

				accounts.push_back(acc);
				return res;
			}
		}

		SIPCall* MakeCall(string uri) throw(pj::Error) {
			SIPAccount* account = PrimaryAccount();
			return MakeCall(uri, account);
		}

		SIPCall* MakeCall(string uri, SIPAccount* account) throw(pj::Error) {
			addTransportSuffix(uri);
			SIPCall *call = new SIPCall(*account);
			CallOpParam prm(true);
			prm.opt.audioCount = 1;
			prm.opt.videoCount = 0;
			call->makeCall(uri, prm);
			account->calls.push_back(call);
			account->onCallEstablished(call);
			return call;
		}

		std::vector<SIPCall*> Calls() {
			std::vector<SIPCall *> calls;
			BOOST_FOREACH(SIPAccount* acc, accounts) {
				auto account_calls = acc->calls;
				calls.insert(std::end(calls), std::begin(account_calls), std::end(account_calls));
			}
			return calls;
		}

		SIPCall* CallById(int call_id) {
			BOOST_FOREACH(SIPCall* c, Calls()) {
				if (c->getId() == call_id)
					return c;
			}
			return nullptr;
		}

		void HoldOtherCalls(SIPCall* call) {
			auto all_calls = Calls();
			all_calls.erase(std::remove(all_calls.begin(), all_calls.end(), call), all_calls.end());
			BOOST_FOREACH(SIPCall* c, all_calls) {
				c->HoldCall();
			}
		}

		void Hangup(SIPCall* call) {
			call->Hangup();
		}

		void HangupAllCalls() {
			endpoint->hangupAllCalls();
		}
	};

}
#endif
