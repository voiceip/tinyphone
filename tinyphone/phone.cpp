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
		PJ_LOG(4, (__FILENAME__, "Refreshed Device Count: %d", (int)devices.size()));
		if (ApplicationConfig.useDefaultAudioDevice) {
			input_audio_dev = 0;
			output_audio_dev = 0;
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

}
