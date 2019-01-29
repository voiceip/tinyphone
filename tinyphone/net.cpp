#include "stdafx.h"
#include "net.h"
#include <curl/curl.h>
#include <boost/asio.hpp>
#include <fstream>

namespace tp {

	size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
	{
		((std::string*)userp)->append((char*)contents, size * nmemb);
		return size * nmemb;
	}

	size_t CurlHeadersCallback(void *buffer, size_t tSize, size_t tCount, void *userp) {
		char *d = (char*)buffer;
		auto *pHeaders = (std::vector<std::pair<std::string, std::string>> *)(userp);

		int result = 0;

		size_t length = tSize * tCount, index = 0;
		while (index < length)
		{
			unsigned char *temp = (unsigned char *)buffer + index;
			if ((temp[0] == '\r') || (temp[0] == '\n'))
				break;
			index++;
		}

		if (pHeaders != NULL) {
			std::string str((unsigned char*)buffer, (unsigned char*)buffer + index);
			size_t pos = str.find(": ");
			if (pos != std::string::npos)
				pHeaders->push_back(std::make_pair(str.substr(0, pos), str.substr(pos + 2)));
		}
		return tCount;
	}

	tp::HttpResponse url_get_contents(std::string url) throw (std::exception) {
		CURL *curl;
		tp::HttpResponse response;

		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); //5s timeout

			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CurlHeadersCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &response.headers);

			auto res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				response.error = curl_easy_strerror(res);
				std::cerr << "curl_easy_perform() failed: " << response.error << std::endl;
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

	std::string local_ip_address() {
		boost::asio::io_service io_service;
		boost::asio::ip::tcp::resolver resolver(io_service);

		std::string _ip_address = "127.0.0.1";

		std::string h = boost::asio::ip::host_name();
		std::cout << "Host name is " << h << std::endl;
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

	bool is_tcp_port_in_use(unsigned short port) {
		boost::asio::io_service svc;
		boost::asio::ip::tcp::acceptor a(svc);

		boost::system::error_code ec;
		a.open(boost::asio::ip::tcp::v4(), ec) || a.bind({ boost::asio::ip::tcp::v4(), port }, ec);

		return ec == boost::asio::error::address_in_use;
	}

	bool is_udp_port_in_use(unsigned short port) {

		boost::asio::io_service svc;
		boost::asio::ip::udp::socket a(svc);

		boost::system::error_code ec;
		a.open(boost::asio::ip::udp::v4(), ec) || a.bind({ boost::asio::ip::udp::v4(), port }, ec);

		return ec == boost::asio::error::address_in_use;
	}

}
