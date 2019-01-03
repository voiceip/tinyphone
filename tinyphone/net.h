#pragma once

#ifndef NET_HEADER_FILE_H
#define NET_HEADER_FILE_H

#include <iostream>
#include <string>
#include <vector>

namespace tp {

	struct HttpResponse {
		long code;
		std::string body;
		std::vector<std::pair<std::string, std::string>> headers;
		std::string error;
	};

	tp::HttpResponse url_get_contents(std::string url) throw (std::exception);

	std::string file_get_contents(std::string  const& path) throw (std::exception);

	std::string local_ip_address();

	bool is_tcp_port_in_use(unsigned short port);

	bool is_udp_port_in_use(unsigned short port);

}

#endif