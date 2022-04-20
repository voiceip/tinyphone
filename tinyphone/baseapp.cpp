#include "stdafx.h"

#include "server.h"
#include "utils.h"
#include "net.h"
#include "consts.h"
#include "config.h"
#include "log.h"
#include "app.hpp"
#include "tpendpoint.h"

#include <boost/foreach.hpp>
#include <algorithm>

using namespace std;
using namespace pj;

#define MAX_DNS_SERVERS 4


inline void  pj_logerror(pj_status_t status, char * message) {
    if (status != PJ_SUCCESS) {
        CROW_LOG_ERROR << "pjsua returned error : " << status;
    }
}

namespace tp {

    tm* launchDate;
    std::string sipLogFile, httpLogFile;
    TinyPhoneHttpServer* tpHttpServer;
    tp::Endpoint ep;

    void InitPJSUAEndpoint(std::string logfile) {
        /* Create endpoint instance! */
        try {
            ep.libCreate();

            // Init library
            EpConfig ep_cfg;
            ep_cfg.logConfig.fileFlags = PJ_O_APPEND;
            ep_cfg.logConfig.filename = logfile;
            ep_cfg.logConfig.consoleLevel = ApplicationConfig.pjLogLevel;
            ep_cfg.logConfig.msgLogging = true;
            ep_cfg.logConfig.level = ApplicationConfig.pjLogLevel;
            ep_cfg.logConfig.decor |= PJ_LOG_HAS_CR | PJ_LOG_HAS_DAY_OF_MON |  PJ_LOG_HAS_MONTH |  PJ_LOG_HAS_YEAR ;
            ep_cfg.uaConfig.userAgent = ApplicationConfig.ua();
            ep_cfg.uaConfig.threadCnt = ApplicationConfig.pjThreadCount;
            if (ApplicationConfig.enableSTUN) {
                ep_cfg.uaConfig.stunServer = ApplicationConfig.stunServers;
            }
            ep_cfg.medConfig.threadCnt = ApplicationConfig.pjMediaThreadCount;
            ep_cfg.medConfig.noVad = ApplicationConfig.disableVAD;
            ep_cfg.medConfig.clockRate = ApplicationConfig.clockRate;

            if (ApplicationConfig.enableNoiseCancel) {
                ep_cfg.medConfig.ecTailLen = ApplicationConfig.ecTailLen;
                ep_cfg.medConfig.ecOptions = PJMEDIA_ECHO_DEFAULT | PJMEDIA_ECHO_USE_NOISE_SUPPRESSOR;
            } else {
                ep_cfg.medConfig.ecTailLen = 0;
            }

            ep.libInit(ep_cfg);

            // Transport Setup
            TransportConfig tcfg;
            int port = 5060;

            switch (ApplicationConfig.transport)
            {
            case PJSIP_TRANSPORT_UDP:
                while (is_udp_port_in_use(port)) {
                    port++;
                }
                break;
            case PJSIP_TRANSPORT_TCP:
            case PJSIP_TRANSPORT_TLS:
                while (is_tcp_port_in_use(port)) {
                    port++;
                }
                break;
            default:
                break;
            }

            std::string productVersion;
            GetProductVersion(productVersion);

            CROW_LOG_INFO << "Running Product Version: " << productVersion;
            CROW_LOG_INFO << "Using Transport Protocol: " << ApplicationConfig.transport;
            CROW_LOG_INFO << "Using Transport Port: " << port;
            CROW_LOG_INFO << "Using UA: " << ApplicationConfig.ua();
            
            tcfg.port = port;
            auto status = ep.transportCreate((pjsip_transport_type_e)ApplicationConfig.transport, tcfg);
            if (status != PJ_SUCCESS) {
                CROW_LOG_INFO << "pjsua.transportCreate returned status : "  << status ;
            }

            // Configure the DNS resolvers to also handle SRV records
            pjsip_endpoint *endpt = pjsua_get_pjsip_endpt();
            std::vector<std::string> dnsServers = GetLocalDNSServers();
            pj_dns_resolver* resolver;
            pj_logerror(pjsip_endpt_create_resolver(endpt, &resolver),"pjsip_endpt_create_resolver");

            struct pj_str_t servers[MAX_DNS_SERVERS];
            int dnsServersCount = static_cast<int>(dnsServers.size());
            for (unsigned int i = 0; i < std::min(dnsServersCount, MAX_DNS_SERVERS); ++i) {
                pj_cstr(&servers[i], dnsServers.at(i).c_str());
            }

            pj_dns_resolver_set_ns(resolver, dnsServers.size(), servers, NULL);
            pjsip_endpt_set_resolver(endpt, resolver);
            CROW_LOG_INFO << "PJSUA2 Set DNS Resolvers Done... : " << dnsServers.size();
        }
        catch (Error & err) {
            CROW_LOG_ERROR << "Exception: " << err.info();
            exit(1);
        }

        /* Initialization is done, now start pjsua */
        try {
            ep.libStart();
            CROW_LOG_INFO  << "PJSUA2 Started...";
        }
        catch (Error & err) {
            CROW_LOG_ERROR << "Exception: " << err.info() ;
            exit(1);
        }
    }


    void StartApp(){
        
        std:cout << "Starting Tinyphone...." << std::endl;

        InitConfig();

        tp::launchDate = now();
        tp::sipLogFile = GetLogFile(SIP_LOG_FILE, "log");
        tp::httpLogFile = GetLogFile(HTTP_LOG_FILE, "log");
        tp::tpHttpServer = new TinyPhoneHttpServer(&ep, tp::httpLogFile);
        tp::tpHttpServer->Configure();

        InitPJSUAEndpoint(tp::sipLogFile);
        
        tp::tpHttpServer->Start();
    }

    void StopApp(){
        CROW_LOG_INFO << "Stopping Tinyphone....";
        if(tp::tpHttpServer != nullptr)
            tp::tpHttpServer->Stop();
    }

    TinyPhone* GetPhone(){
        return tp::tpHttpServer->tinyPhone;
    }
    
}
