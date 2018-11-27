#pragma once

#include <pj/assert.h>
#include <pj/pool.h>
#include <pj/string.h>
#include <string.h>
#include <pjsua-lib/pjsua.h>
#include <crow.h>
#include <windows.h>
#include <stdio.h>

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)

void print_thread_name()
{
	std::thread::id this_id = std::this_thread::get_id();
	CROW_LOG_INFO << "Thread id: #" << this_id ;
}

PJ_IDEF(pj_str_t) pj_str(std::string str)
{
	pj_str_t dst;	
	dst.ptr = (char *) str.c_str();
	dst.slen =  str.length();
	return dst;	
}

void PrintErr(std::string message) {
	HANDLE hConsoleErr = GetStdHandle(STD_ERROR_HANDLE);
	SetConsoleTextAttribute(hConsoleErr, FOREGROUND_RED);
	fprintf(stderr, "%s\n", message.c_str());
	SetConsoleTextAttribute(hConsoleErr, FOREGROUND_WHITE);

}

/*
* pj_thread_auto_register(void)
* Provided by Jim Gomes on pjsip forum
* Note, here we use calloc to allocated storage for thread description
* without freeing the descriptors. This leaks 256 bytes per thread
registration.
*/
static PJ_DEF(pj_status_t) pj_thread_auto_register(void)
{
	print_thread_name();

	pj_status_t rc;

	if (!pj_thread_is_registered())
	{
		pj_thread_desc *p_thread_desc;
		pj_thread_t* thread_ptr;
		p_thread_desc = (pj_thread_desc *)calloc(1,sizeof(pj_thread_desc));
		rc = pj_thread_register("auto_thr%p", *p_thread_desc, &thread_ptr);
	}
	else
	{
		rc = PJ_SUCCESS;
	}
	return rc;
}

bool tcp_port_in_use(unsigned short port) {
	using namespace boost::asio;
	using ip::tcp;

	io_service svc;
	tcp::acceptor a(svc);

	boost::system::error_code ec;
	a.open(tcp::v4(), ec) || a.bind({ tcp::v4(), port }, ec);

	return ec == error::address_in_use;
	 
}

bool udp_port_in_use(unsigned short port) {
	using namespace boost::asio;
	using ip::udp;

	io_service svc;
	udp::socket a(svc);

	boost::system::error_code ec;
	a.open(udp::v4(), ec) || a.bind({ udp::v4(), port }, ec);

	return ec == error::address_in_use;

}