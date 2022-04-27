// console.cpp : Defines the entry point for the console application.

// #include "stdafx.h"
#include <stdio.h>
#include <ctime>
#include <algorithm>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <resolv.h>
#include <netdb.h>
#include <vector>
#include <iostream>

#include "server.h"
#include "utils.h"
#include "net.h"
#include "consts.h"
#include "config.h"
#include "log.h"
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
                dnsServers.push_back(str);

            } else if (family == AF_INET6) { // IPV6 address
                char str[INET6_ADDRSTRLEN]; // String representation of address
                inet_ntop(AF_INET6, &(res->nsaddr_list [i].sin_addr.s_addr), str, INET6_ADDRSTRLEN);
            }
        }
        res_nclose(res);
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


int main(int argc, char *argv[])
{
    std::cout << "Hello World!\n";
    Start();
    return 0;
}