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

	static size_t CurlReadCallback(void *dest, size_t size, size_t nmemb, void *userp)
	{
		struct CurlPostData *wt = (struct CurlPostData *)userp;
		size_t buffer_size = size*nmemb;
		
		if(wt->sizeleft) {
			/* copy as much as possible from the source to the destination */ 
			size_t copy_this_much = wt->sizeleft;
			if(copy_this_much > buffer_size)
			copy_this_much = buffer_size;
			memcpy(dest, wt->readptr, copy_this_much);
		
			wt->readptr += copy_this_much;
			wt->sizeleft -= copy_this_much;
			return copy_this_much; /* we copied this many bytes */ 
		}
		
		return 0; /* no more data left to deliver */ 
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

	tp::HttpResponse http_post(std::string url, std::string body) throw (std::exception) {
		CURL *curl;
		tp::HttpResponse response;


		curl = curl_easy_init();
		if (curl) {

			struct curl_slist *request_headers = NULL;

			struct CurlPostData payload; 
			payload.readptr = body.c_str();
 			payload.sizeleft = body.length();

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); //5s timeout

			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CurlHeadersCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &response.headers);


			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			/* we want to use our own read function */ 
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, CurlReadCallback);
			/* pointer to pass to our read function */ 
			curl_easy_setopt(curl, CURLOPT_READDATA, &payload);
		
			/*
			If you use POST to a HTTP 1.1 server, you can send data without knowing
			the size before starting the POST if you use chunked encoding. You
			enable this by adding a header like "Transfer-Encoding: chunked" with
			CURLOPT_HTTPHEADER. With HTTP 1.0 or without chunked transfer, you must
			specify the size in the request.
			*/ 
			#ifdef USE_CHUNKED
				request_headers = curl_slist_append(request_headers, "Transfer-Encoding: chunked");
			#else
				/* Set the expected POST size. If you want to POST large amounts of data,
				consider CURLOPT_POSTFIELDSIZE_LARGE */ 
				curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)payload.sizeleft);
			#endif
			
			#ifdef DISABLE_EXPECT
				/*
				Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue"
				header.  You can disable this header with CURLOPT_HTTPHEADER as usual.
				NOTE: if you want chunked transfer too, you need to combine these two
				since you can only set one list of headers with CURLOPT_HTTPHEADER. */ 
			
				/* A less good option would be to enforce HTTP 1.0, but that might also
				have other implications. */ 
				request_headers = curl_slist_append(request_headers, "Expect:");
			#endif

			request_headers = curl_slist_append(request_headers, "Accept: application/json");
			request_headers = curl_slist_append(request_headers, "Content-Type: application/json");

			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, request_headers);
			
			auto res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				response.error = curl_easy_strerror(res);
				std::cerr << "http_post curl_easy_perform() failed: " << response.error << std::endl;
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

	tp::HttpResponse http_get(std::string url) throw (std::exception) {
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

	std::string urldecode(std::string data){
		std::string out = data;
		CURL *curl = curl_easy_init();
		if(curl) {
			char *decoded = curl_easy_unescape(curl,data.c_str(), data.length(), NULL);
			if(decoded) {
				out = std::string(decoded);
				curl_free(decoded);
			}
			curl_easy_cleanup(curl);
		}
		return out;
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
