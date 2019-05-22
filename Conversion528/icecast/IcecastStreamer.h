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

struct ContentToStream
{
	std::vector<std::string> playlist;

	ContentToStream() = default;

	ContentToStream(const std::string& singleSong)
		: playlist( { singleSong } )
	{
	}

	ContentToStream(const std::vector<std::string>& newPlaylist)
		: playlist (newPlaylist)
	{
	}

};

struct Uploading
{
	ContentToStream contentToStream;
	std::string addres;
	std::string port;
};


class IcecastStreamer
{
public:
	enum class AudioFormat
	{
		Invalid,
		WAV,
		MP3,
		AAC,
		M4A
	};

	std::string addres;
	std::string port;

	boost::asio::io_service& io_service;

	IcecastStreamer(boost::asio::io_service& ioService, std::string addres, std::string port);

	void streamFile(const ContentToStream& contentToStream, std::shared_ptr<std::promise<void>> promise);
	void streamFile(boost::asio::ip::tcp::endpoint endpoint, const ContentToStream& contentToStream, std::shared_ptr<std::promise<void>> promise);

	bool streamFileInner(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const Uploading& uploading);
};

#endif