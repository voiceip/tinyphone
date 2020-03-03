#pragma once

#ifndef ACCOUNT_HEADER_FILE_H
#define ACCOUNT_HEADER_FILE_H

#include <pjsua2.hpp>
#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include "enum.h"
#include "events.h"
#include "call.h"
#include "utils.h"
#include "config.h"
#include <boost/foreach.hpp>


using namespace std;
using namespace pj;

namespace tp {
	
	class TinyPhone;

	class SIPAccount : public Account
	{
		std::string account_name;
		std::promise<int> create_result_promise;
		int create_result_promise_fullfilled = 0;

		tp::TinyPhone* phone;
		
	public:

		AccountConfig accConfig;
		EventStream* eventStream;
		std::string domain;
		std::vector<SIPCall *> calls;

		SIPAccount(tp::TinyPhone* parent, std::string name, EventStream* es, AccountConfig cfg)
		{
			account_name = name;
			eventStream = es;
			phone = parent;
			accConfig = cfg;
		}

		~SIPAccount()
		{
			pj_thread_auto_register();
			std::cout << "Account is being deleted:" << account_name << ". No of calls : " << calls.size() << std::endl;;
			try {
				if (pjsua_acc_is_valid(getId()) != 0) {
					AccountInfo ai = getInfo();
					OnRegStateParam prm{
						200,
						pjsip_status_code::PJSIP_SC_OK
					};
					ai.regIsActive = false;
					eventStream->publishEvent(ai, prm);
				}
				else {
					std::cout << "Underlying Account Already Shutdown" << account_name << std::endl;
				}
			}
			catch (...) {
				std::cerr << "Account Shutdown Error" << account_name << std::endl;
			};
			shutdown();
		}

		std::string Name();

		/**
		* removes the call from the account list. 
		* Doesn't delete it, you must delete the reference of it.
		*/
		void removeCall(SIPCall *call);

		void reRegister();

		void UnRegister();

		virtual void onRegState(OnRegStateParam &prm);

		std::future<int> Create(const pj::AccountConfig &cfg, bool make_default = false) throw(Error);

		void HoldAllCalls();

		virtual void onCallEstablished(SIPCall *call) ;

		virtual void onCallEnd(SIPCall *call) ;

		virtual void onIncomingCall(OnIncomingCallParam &iprm);

	};

}

#endif
