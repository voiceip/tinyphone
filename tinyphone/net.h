#pragma once

#include "stdafx.h"
#include <crow.h>
#include <iostream>
#include <string>
#include <curl/curl.h>

using namespace boost;
using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

namespace tp {
	struct HttpResponse {
		long code;
		std::string body;
		std::vector<std::pair<std::string, std::string>> headers;
	};
}

static size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

static tp::HttpResponse file_get_contents(std::string url) throw (std::exception) {
	CURL *curl;
	tp::HttpResponse response;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); //10s timeout

		auto res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		}
		else {
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.code);
		}
		curl_easy_cleanup(curl);
	}
	else {
		throw std::runtime_error("Unable to init curl libary");
	}
	return response;
}


static std::string local_ip_address() {
	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);

	std::string _ip_address = "127.0.0.1";

	std::string h = boost::asio::ip::host_name();
	std::cout << "Host name is " << h << '\n';
	std::cout << "IP addresses are: ";
	std::for_each(resolver.resolve({ h, "" }), {}, [&_ip_address](const auto& re) {
		boost::asio::ip::address address = re.endpoint().address();
		std::cout << address << " ";
		if (!address.is_v6()) {
			_ip_address = address.to_string();
		}
	});
	std::cout << std::endl;
	return _ip_address;
}

static bool is_tcp_port_in_use(unsigned short port) {
	io_service svc;
	tcp::acceptor a(svc);

	boost::system::error_code ec;
	a.open(tcp::v4(), ec) || a.bind({ tcp::v4(), port }, ec);

	return ec == error::address_in_use;
}

static bool is_udp_port_in_use(unsigned short port) {
 
	io_service svc;
	udp::socket a(svc);

	boost::system::error_code ec;
	a.open(udp::v4(), ec) || a.bind({ udp::v4(), port }, ec);

	return ec == error::address_in_use;

}
 