#pragma once

#include "stdafx.h"
#include <crow.h>
#include <iostream>

using namespace boost;
using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

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
 