// console.cpp : Defines the entry point for the console application.

#include <stdio.h>
#include <ctime>
#include <algorithm>
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


void Start(){
   tp::StartApp();
   exit(0);
}

void Stop(){
    tp::StopApp();
}

int main(int argc, char *argv[])
{
    std::cout << "Hello Tinyphone!\n";
    Start();
    return 0;
}
