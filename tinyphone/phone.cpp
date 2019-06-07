#include "stdafx.h"
#include "phone.h"

namespace tp {

	void from_json(const nlohmann::json& j, AccountConfig& p) {
		j.at("username").get_to(p.username);
		j.at("domain").get_to(p.domain);
		j.at("password").get_to(p.password);

		if (j.find("proxy") != j.end()) {
			j.at("proxy").get_to(p.proxy);
		}
	}

	void TinyPhone::ConfigureAudioDevices(){
		AudDevManager& audio_manager = Endpoint::instance().audDevManager();
		audio_manager.refreshDevs();
		AudioDevInfoVector devices = audio_manager.enumDev();
		PJ_LOG(3, (__FILENAME__, "Refreshed Device Count: %d", (int)devices.size()));
		int dev_idx = 0;
		BOOST_FOREACH(AudioDevInfo* info, devices) {
			PJ_LOG(3, (__FILENAME__, "Found Device #%d %s", dev_idx++, info->name.c_str()));
		}
		if (ApplicationConfig.useDefaultAudioDevice) {
			PJ_LOG(3, (__FILENAME__, "Using Default Audio Device(s)"));
			input_audio_dev = -1;
			output_audio_dev = -2;
		} else {
			BOOST_FOREACH(string& search_string, ApplicationConfig.prefferedAudioDevices) {
				int dev_idx = 0;
				BOOST_FOREACH(AudioDevInfo* info, devices) {
					string dev_name = info->name;
					boost::to_lower(dev_name);
					if (dev_name.find(search_string) != string::npos) {
						if (info->inputCount > 0 && input_audio_dev <= 0) {
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
		audio_manager.setCaptureDev(input_audio_dev);
		audio_manager.setPlaybackDev(output_audio_dev);
	}

	void TinyPhone::refreshDevices() {
		PJ_LOG(4, (__FILENAME__, "Refreshing Audio Devices"));
		ConfigureAudioDevices();
	}

	std::future<int> TinyPhone::AddAccount(AccountConfig& config) throw (std::exception) {
			string account_name = SIP_ACCOUNT_NAME(config.username, config.domain);
			synchronized(add_acc_mutex){
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
		}

}
