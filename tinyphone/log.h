#pragma once

#ifndef LOG_HEADER_FILE_H
#define LOG_HEADER_FILE_H

#include <iostream>
#include <ctime>

namespace tp {
	extern std::string sipLogFile;
	extern std::string httpLogFile;
	extern tm* launchDate;
}

#endif