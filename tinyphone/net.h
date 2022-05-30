#pragma once

#ifndef NET_HEADER_FILE_H
#define NET_HEADER_FILE_H

#include <iostream>
#include <string>
#include <vector>

namespace tp {

	struct CurlPostData {
		const char *readptr;
		size_t sizeleft;
	};

	struct HttpResponse {
		long code;
		std::string body;
		std::vector<std::pair<std::string, std::string>> headers;
		std::string error;
	};

	tp::HttpResponse http_get(std::string url) throw (std::exception);

	tp::HttpResponse http_post(std::string url, std::string body) throw (std::exception);

    std::string urldecode(std::string data);

	std::string local_ip_address();

	bool is_tcp_port_in_use(unsigned short port);

	bool is_udp_port_in_use(unsigned short port);

}

#endif
