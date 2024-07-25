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
            arrInfo.count = (int) accounts.size();
            if (accounts.size() < 10 ) {
                std::copy(accounts.begin(), accounts.end(), arrInfo.accounts);
            }
        } catch(...) {
            //do nothing
        }
    }
    return arrInfo;
}

