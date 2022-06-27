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

#include <map> 


namespace tp {

    enum RingTune
    {   IncomingRing = 0,
        RingBack = 1
    };

	class TinyPhone
	{
		typedef std::map<string, SIPAccount*> map_string_acc;
		map_string_acc accounts;
		pj::Endpoint* endpoint;
		EventStream* eventStream;

		std::string userConfigFile;

	private:
	    std::recursive_mutex add_acc_mutex;
        ToneGenerator* ringingTone;
        ToneGenerator* ringbackTone;
        int ringing_count;
        int ringback_count;

		std::string addTransportSuffix(std::string &str) {
			tp::AddTransportSuffix(str, ApplicationConfig.transport);
			return str;
		}

		bool InitOptionsModule();

	public:
		int input_audio_dev = 0, output_audio_dev = 0;

		TinyPhone(pj::Endpoint* ep) {
			endpoint = ep;

			boost::filesystem::path tiny_dir = GetAppDir();
			auto logfile = tiny_dir.append("user.conf");
			userConfigFile = logfile.string();
			std::cout << "TinyPhone userConfigFile: " << userConfigFile << std::endl;
			Initialize();
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

		void CreateEventStream(channel<std::string>* ch) {
			eventStream = new EventStream(ch);
		}

		void SetCodecs();

		void EnableCodec(std::string codec_name, pj_uint8_t priority);

		bool TestAudioDevice();

		void ConfigureAudioDevices();

		void refreshDevices();

		void InitMetricsClient();

		std::vector<SIPAccount *> Accounts();

		bool HasAccounts() ;

		bool RestoreAccounts();

		bool SaveAccounts();

		SIPAccount* PrimaryAccount();

		SIPAccount* AccountByName(string name) ;

		void Logout(SIPAccount* acc) throw(pj::Error);

		int Logout() throw(pj::Error) ;

		void EnableAccount(SIPAccount* account) throw (std::exception) ;

		std::future<int> AddAccount(AccountConfig& config) throw (std::exception);
        
        void AddDefaultAccount() throw (std::exception);

		SIPCall* MakeCall(string uri) throw(pj::Error) ;

		SIPCall* MakeCall(string uri, SIPAccount* account) throw(pj::Error);

		std::vector<SIPCall*> Calls();

		SIPCall* CallById(int call_id) ;

		void HoldOtherCalls(SIPCall* call);

		void Answer(SIPCall* call);
		void Hangup(SIPCall* call);

		bool Conference(SIPCall* call);
		bool BreakConference(SIPCall* call);

		void HangupAllCalls();

		bool Initialize();
		void Shutdown();
		
		void StartRinging(SIPCall* call, RingTune tune = IncomingRing);
		void StopRinging(SIPCall* call);
	};


}
#endif
