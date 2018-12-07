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

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

class TinyPhone
{
	std::vector<SIPAccount*> accounts;
	pj::Endpoint* endpoint;
	EventStream* eventStream;

public:
	int input_audio_dev = 0, output_audio_dev = 0;

	TinyPhone(pj::Endpoint* ep) {
		endpoint = ep;
	}

	~TinyPhone() {
		pj_thread_auto_register();
		std::cout << "Shutting Down TinyPhone" << std::endl;
		HangupAllCalls();
		Logout();
		delete eventStream;
		std::cout << "Shutting Down TinyPhone Complete" << std::endl;
	}

	void SetCodecs() {
		const pj_str_t ID_ALL = { "*", 1 };
		pjsua_codec_set_priority(&ID_ALL, PJMEDIA_CODEC_PRIO_DISABLED);	
		auto allowed_codecs = splitString(SIP_ALLOWED_AUDIO_CODECS, ' ');
		BOOST_FOREACH(std::string codec, allowed_codecs){
			EnableCodec(codec);
		}
	}

	void CreateEventStream(channel<std::string>* ch) {
		eventStream = new EventStream(ch);
	}

	void EnableCodec(std::string codec_name) {
		auto codec = pj_str(codec_name);
		auto status = pjsua_codec_set_priority(&codec, PJMEDIA_CODEC_PRIO_NORMAL);
		if (status == PJ_SUCCESS)
			PJ_LOG(3, (__FILENAME__, "%s activated", codec.ptr));
		else
			PJ_LOG(3, (__FILENAME__, "Failed activating %s, err=%d", codec.ptr, status));
		free(codec.ptr);
	}

	void ConfigureAudioDevices(int input_device = -1, int output_device = -1) {
		AudDevManager& audio_manager = Endpoint::instance().audDevManager();
		if (input_device == -1 || output_device == -1) {
			audio_manager.refreshDevs();
			AudioDevInfoVector devices = audio_manager.enumDev();
			vector<string> preffered_devices{ "sound", "usb" , "headphone", "audio" , "microphone" , "speakers" };
			BOOST_FOREACH(string& search_string, preffered_devices) {
				int dev_idx = 0;
				BOOST_FOREACH(AudioDevInfo* info, devices) {
				string dev_name = info->name;
				boost::to_lower(dev_name);
					if (dev_name.find(search_string) != string::npos) {
						if (info->inputCount > 0 && input_audio_dev <= 0 ) {
							input_audio_dev = dev_idx;
							PJ_LOG(3, (__FILENAME__, "Selected Input #%d %s", dev_idx, info->name.c_str()));
						}
						if (info->outputCount > 0 && output_audio_dev <= 0) {
							output_audio_dev = dev_idx;
							PJ_LOG(3, (__FILENAME__, "Selected Output #%d %s", dev_idx, info->name.c_str()));
						}
					}
					dev_idx++;
				}
			}
		}
		else {
			input_audio_dev = input_device;
			output_audio_dev = output_device;
		}
		audio_manager.setCaptureDev(input_audio_dev);
		audio_manager.setPlaybackDev(output_audio_dev);
	}

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

	SIPAccount* AccountByURI(string uri) {
		if (!HasAccounts())
			return nullptr;
		else {
			string full_uri = "sip:" + uri;
			SIPAccount* account = nullptr;
			BOOST_FOREACH(SIPAccount* acc, accounts) {
				if (acc->getInfo().uri == full_uri){
					account = acc;
					break;
				}
			}
			return account;
		}
	}

	void Logout() {
		auto it = accounts.begin();
		while (it != accounts.end()) {
			delete (*it);
			it = accounts.erase(it);
		}
	}

	std::future<int> AddAccount(string username, string domain, string password) throw (std::invalid_argument) {
		string account_name = SIP_ACCOUNT_NAME(username, domain);
		auto exits = AccountByURI(account_name);
		if (exits != nullptr) {
			throw std::invalid_argument("Account already exists");
		} else {
			AccountConfig acc_cfg;
			acc_cfg.idUri = ("sip:" + account_name);
			acc_cfg.regConfig.registrarUri = ("sip:" + domain);
			acc_cfg.sipConfig.authCreds.push_back(AuthCredInfo("digest", "*", username, 0, password));

			acc_cfg.regConfig.timeoutSec = SIP_REG_DURATION;
			acc_cfg.regConfig.retryIntervalSec = SIP_REG_RETRY_INTERVAL;
			acc_cfg.regConfig.firstRetryIntervalSec = SIP_REG_FIRST_RETRY_INTERVAL;
			acc_cfg.videoConfig.autoTransmitOutgoing = PJ_FALSE;
			acc_cfg.videoConfig.autoShowIncoming = PJ_FALSE;

			SIPAccount *acc(new SIPAccount(account_name, eventStream));
			acc->domain = domain;
			auto res = acc->Create(acc_cfg);
			
			accounts.push_back(acc);
			return res;
		}
	}

	SIPCall* MakeCall(string uri) throw(pj::Error) {
		SIPAccount* account = PrimaryAccount();
		return MakeCall(uri, account);
	}

	SIPCall* MakeCall(string uri, SIPAccount* account) throw(pj::Error){
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

#endif
