// tinyphone.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <pjsua-lib/pjsua.h>
#include <crow.h>
#include "server.h"
#include "utils.h"

using namespace std;

#pragma comment(lib, "ws2_32.lib") 
//#pragma comment(lib, "libpjproject-i386-Win32-vc14-Release-Static-NoVideo.lib")
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Debug-Static.lib")

#define THIS_FILE	"tinyphone"

#define SIP_DOMAIN	"preprod.fktel.ga"
#define SIP_USER	"alice"
#define SIP_PASSWD	"nopassword"


/* Callback called by the library upon receiving incoming call */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id, pjsip_rx_data *rdata)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    pjsua_call_get_info(call_id, &ci);

    PJ_LOG(3,(THIS_FILE, "Incoming call from %.*s!!",
			 (int)ci.remote_info.slen,
			 ci.remote_info.ptr));

    /* Automatically answer incoming calls with 200/OK */
    pjsua_call_answer(call_id, 200, NULL, NULL);
}

/* Callback called by the library when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(e);

    pjsua_call_get_info(call_id, &ci);
    PJ_LOG(3,(THIS_FILE, "Call %d state=%.*s", call_id,
			 (int)ci.state_text.slen,
			 ci.state_text.ptr));
}

/* Callback called by the library when call's media state has changed */
static void on_call_media_state(pjsua_call_id call_id)
{
    pjsua_call_info ci;

    pjsua_call_get_info(call_id, &ci);

    if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
	// When media is active, connect call to sound device.
	pjsua_conf_connect(ci.conf_slot, 0);
	pjsua_conf_connect(0, ci.conf_slot);
    }
}

/* Display error and exit application */
static void error_exit(const char *title, pj_status_t status)
{
    pjsua_perror(THIS_FILE, title, status);
    pjsua_destroy();
    exit(1);
}

pj_status_t add_account(string user, string domain,  string password, pjsua_acc_id *acc_id) {
	/* Register to SIP server by creating SIP account. */
	pjsua_acc_config cfg;

	pjsua_acc_config_default(&cfg);
	string _id = "sip:" + user + "@" + domain;
	cfg.id = pj_str((char *) _id.c_str());
	string _reg_uri = "sip:" + domain;
	cfg.reg_uri = pj_str((char *)_reg_uri.c_str());
	cfg.cred_count = 1;
	cfg.cred_info[0].realm = pj_str((char *)domain.c_str());
	cfg.cred_info[0].scheme = pj_str("digest");
	cfg.cred_info[0].username = pj_str((char *)user.c_str());
	cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
	cfg.cred_info[0].data = pj_str((char *)password.c_str());
	cfg.reg_retry_interval = 30;
	cfg.reg_first_retry_interval = 15;
	cfg.reg_timeout = 180;
	cfg.vid_out_auto_transmit = PJ_FALSE;
	cfg.vid_in_auto_show = PJ_FALSE;

	if (*acc_id >= 0) {
		cout << "UnRegistering from existing account " << *acc_id << endl;
		pjsua_acc_del(*acc_id);
	}
	return pjsua_acc_add(&cfg, PJ_TRUE, acc_id);
}

int main(int argc, char *argv[])
{
	map<pjsua_acc_id, string> accounts;
    pjsua_acc_id acc_id(-1);
    pj_status_t status;

	crow::App<TinyPhoneMiddleware> app;
	//app.get_middleware<TinyPhoneMiddleware>().setMessage("tinyphone");

    /* Create pjsua first! */
    status = pjsua_create();
    if (status != PJ_SUCCESS) error_exit("Error in pjsua_create()", status);

    /* Init pjsua */
    {
	pjsua_config cfg;
	pjsua_logging_config log_cfg;

	pjsua_config_default(&cfg);
	cfg.cb.on_incoming_call = &on_incoming_call;
	cfg.cb.on_call_media_state = &on_call_media_state;
	cfg.cb.on_call_state = &on_call_state;

	pjsua_logging_config_default(&log_cfg);
	log_cfg.console_level = 3; //4

	status = pjsua_init(&cfg, &log_cfg, NULL);
	if (status != PJ_SUCCESS) error_exit("Error in pjsua_init()", status);
    }

    /* Add UDP transport. */
    {
	pjsua_transport_config cfg;
	pjsua_transport_config_default(&cfg);
	cfg.port = 5060;
	status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, NULL);
	if (status != PJ_SUCCESS) error_exit("Error creating transport", status);
    }

	/* audio device selction */
	pjmedia_aud_dev_index dev_idx;
	int dev_count = pjmedia_aud_dev_count();
	printf("Got %d audio devices\n", dev_count);
	for (dev_idx = 0; dev_idx<dev_count; ++dev_idx) {
		pjmedia_aud_dev_info info;
		status = pjmedia_aud_dev_get_info(dev_idx, &info);
		printf("%d. %s (in=%d, out=%d)\n", dev_idx, info.name, info.input_count, info.output_count);
	}

	/* Set audio device*/
	//pjmedia_aud_dev_index dev_idx = PJMEDIA_AUD_DEFAULT_CAPTURE_DEV;
	//status = pjmedia_aud_dev_default_param(dev_idx, &param)
	

    /* Initialization is done, now start pjsua */
    status = pjsua_start();
    if (status != PJ_SUCCESS) error_exit("Error starting pjsua", status);



	/* Define HTTP Endpoints */
	CROW_ROUTE(app, "/")([]() {
		return "Hello world";
	});

	CROW_ROUTE(app, "/login")
		.methods("POST"_method)
	([&acc_id, &accounts](const crow::request& req) {
		auto x = crow::json::load(req.body);
		if (!x)
			return crow::response(400, "Bad Request");
		string username = x["username"].s();
		string domain = x["domain"].s();
		string password = x["password"].s();
		string account_name = username + "@" + domain;

		pj_thread_auto_register();
		auto status = add_account(username, domain, password, &acc_id);

		if (status != PJ_SUCCESS) {
			return crow::response(500, "Failed to Add Account");
		}
		else {
			CROW_LOG_INFO << ("Registered account " + to_string(acc_id) + " : " + account_name);
			accounts.insert(pair<pjsua_acc_id, string>(acc_id, account_name));
			return crow::response(200, "Account added succesfully");
		}
	});

	CROW_ROUTE(app, "/dial")
		.methods("POST"_method)
	([&acc_id, &accounts](const crow::request& req) {
		auto dial_uri = (char *)req.body.c_str();
	
		if (acc_id < 0 )
			return crow::response(400, "No Account Registed Yet");

		auto account_name = accounts.find(acc_id)->second;

		CROW_LOG_INFO << ("Dial Request to " + req.body + " via account " + account_name);


		try {
			pj_thread_auto_register();

			/* If argument is specified, it's got to be a valid SIP URL
			auto status = pjsua_verify_url(dial_uri);
			if (status != PJ_SUCCESS) {
				 return crow::response(400, "Invalid URL");
			} */

			//TODO: verify or create valid sip uri or this would crash :(

			pj_str_t uri = pj_str(dial_uri);
			pjsua_call_setting settings;
			settings.vid_cnt = 0;

			auto status = pjsua_call_make_call(acc_id, &uri, 0, NULL, NULL, NULL);
			if (status != PJ_SUCCESS) {
				return crow::response(500, "Error making call" + status);
			}
			else {
				return crow::response(200, ("Dialed via "+ account_name));
			}
		}
		catch (std::exception& e) {
			CROW_LOG_ERROR << "Exception catched : " << e.what();
			return crow::response(500, "Something Went Wrong");
		}

	});

	CROW_ROUTE(app, "/logout")
		.methods("POST"_method)
	([&acc_id]() {
		CROW_LOG_INFO << "Logout from  account " << acc_id ;
		try{
			pj_thread_auto_register();
			CROW_LOG_INFO << "Atempting logout " << acc_id;
			auto status = pjsua_acc_del(acc_id);
			if (status != PJ_SUCCESS) {
				return crow::response(500, "Error logging out :" + status);
			}
			else {
				return crow::response(200, "Logged Out");
			}
		} catch (std::exception& e) {
			CROW_LOG_ERROR << "Exception catched : " << e.what();
			return crow::response(500, "Something Went Wrong");
		}
	});

	CROW_ROUTE(app, "/exit")
		.methods("POST"_method)
	([&app](const crow::request& req) {
		CROW_LOG_INFO << "Shutdown Request from client: " << req.body;
		app.stop();
		return "Server shutdown";
	});

	CROW_ROUTE(app, "/hangup_all")
		.methods("POST"_method)
	([]() {
		pj_thread_auto_register();
		pjsua_call_hangup_all();
		return "Hangup Calls";
	});		

	app.loglevel(crow::LogLevel::Info);
	//crow::logger::setHandler(std::make_shared<TinyPhoneHTTPLogHandler>());	

	app.port(6060)
		//.multithreaded()
		.run();

	std::cout << "Server has been shutdown... Will Exit now...." << std::endl;
		 
	/* Destroy pjsua */
	pjsua_destroy();

    return 0;
}

