#ifndef FILE_UPLOADER
#define FILE_UPLOADER

#include <iostream>
#include <memory>
#include <mutex>
#include "boost/asio.hpp"
#include "boost/filesystem.hpp"

//OpenSSL stuff -- Vladislav Khorev vladislav.khorev@fishrungames.com
//#define SSL_R_SHORT_READ 219
//#include "ssl/ssl_locl.h"
//#include <boost/asio/ssl.hpp>

#if defined(close)
#undef close
#endif

#if defined(open)
#undef open
#endif

struct Uploading
{
	std::string fileName;
	std::string addres;
	std::string port;
};

class IcecastStreamer
{
public:
	std::string addres;
	std::string port;

	boost::asio::io_service& io_service;

	IcecastStreamer(boost::asio::io_service& ioService, std::string addres, std::string port);

	void streamFile(const std::string& fileName);

private:

	void streamFile(std::shared_ptr<boost::asio::ip::tcp::endpoint> ep, const std::string& fileName);
	bool streamFile(std::shared_ptr<boost::asio::ip::tcp::socket> httpSocket, const std::string& fileName);
};

#endif