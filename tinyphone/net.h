#pragma once

#include "stdafx.h"
#include <crow.h>
#include <iostream>

using namespace boost;
using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

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
 