//
//  osxapp.cpp
//  Tinyphone
//
//  Created by Kinshuk  Bairagi on 03/11/20.
//  Copyright © 2020 Kinshuk  Bairagi. All rights reserved.
//

#include <stdio.h>
#include <ctime>
#include <algorithm>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <resolv.h>
#include <dns.h>
#include <vector>

#include <boost/foreach.hpp>

#include "server.h"
#include "utils.h"
#include "net.h"
#include "consts.h"
#include "config.h"
#include "log.h"
#include "osxapp.h"
#include "app.hpp"
#include "tpendpoint.h"

using namespace std;
using namespace pj;
using namespace tp;

namespace tp {

    std::vector<std::string> GetLocalDNSServers() {
        std::vector <std::string> dnsServers;
        // Get native iOS System Resolvers
        res_ninit(&_res);
        res_state res = &_res;

        for (int i = 0; i < res->nscount; i++) {
            sa_family_t family = res->nsaddr_list[i].sin_family;
            int port = ntohs(res->nsaddr_list[i].sin_port);
            if (family == AF_INET) { // IPV4 address
                char str[INET_ADDRSTRLEN]; // String representation of address
                inet_ntop(AF_INET, & (res->nsaddr_list[i].sin_addr.s_addr), str, INET_ADDRSTRLEN);
                // std::cout << "DNS: " << str << std::endl ;
                dnsServers.push_back(str);

            } else if (family == AF_INET6) { // IPV6 address
                char str[INET6_ADDRSTRLEN]; // String representation of address
                inet_ntop(AF_INET6, &(res->nsaddr_list [i].sin_addr.s_addr), str, INET6_ADDRSTRLEN);
            }
        }
        res_ndestroy(res);
        return dnsServers;
    }
}

void Start(){
   tp::StartApp();
   exit(0);
}

void Stop(){
    tp::StopApp();
}

char* StringtoCharPtr(std::string str){
    char* s = new char[str.size() + 1]{};
    std::copy(str.begin(), str.end(), s);
    return s;
}

struct UIAccountInfoArray Accounts(){
    UIAccountInfoArray arrInfo;
    arrInfo.count = 0;
    if(tp::GetPhone() != nullptr){
        pj_thread_auto_register();
        try{
            auto tpAccounts = tp::GetPhone()->Accounts();
            std::vector<UIAccountInfo> accounts;
            BOOST_FOREACH(tp::SIPAccount* account, tpAccounts) {
                UIAccountInfo acc;
                acc.name =  StringtoCharPtr(account->Name());
                acc.status = StringtoCharPtr(account->getInfo().regStatusText);
                acc.primary = account->isDefault();
                acc.active = account->getInfo().regIsActive;
                accounts.push_back(acc);
            }
            arrInfo.count = accounts.size();
            if (accounts.size() < 10 ) {
                std::copy(accounts.begin(), accounts.end(), arrInfo.accounts);
            }
        } catch(...) {
            //do nothing
        }
    }
    return arrInfo;
}

