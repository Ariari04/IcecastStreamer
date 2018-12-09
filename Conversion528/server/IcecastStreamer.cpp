#include "IcecastStreamer.h"

#include <boost/asio.hpp>
#include <WaveFile.h>

const long long MAX_UPLOADED_FILE_SIZE = 1024 * 1024 * 400;

//bool verify_certificate(bool preverified,
//	boost::asio::ssl::verify_context& ctx)
//{
//	//return preverified;
//
//	char subject_name[256];
//	X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
//	X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
//	std::cout << "Verifying " << subject_name << "\n";
//
//	return true;
//}

IcecastStreamer::IcecastStreamer(boost::asio::io_service& ioService, std::string addres, std::string port)
	: io_service(ioService)
{
	this->addres = addres;
	this->port = port;
}


void IcecastStreamer::streamFile(const std::string& fileName)
{
	boost::asio::ip::tcp::resolver resolver(io_service);

	boost::asio::ip::tcp::resolver::query query(addres, port);
	boost::asio::ip::tcp::resolver::iterator iter;

	try
	{
		iter = resolver.resolve(query);
	}
	catch (std::exception& e)
	{
		std::cout << "IcecastStreamer::postUploadFileHttp: couldn't resolve addres, retry..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		io_service.post([fileName, this]() { streamFile(fileName); });
		return;
	}

	auto ep = std::make_shared<boost::asio::ip::tcp::endpoint>(*iter);

	io_service.post([ep, fileName, this]()
	{
		streamFile(ep, fileName);
	});
}

void IcecastStreamer::streamFile(std::shared_ptr<boost::asio::ip::tcp::endpoint> ep, const std::string& fileName)
{
	std::shared_ptr<boost::asio::ip::tcp::socket> httpSocket = std::make_shared<boost::asio::ip::tcp::socket>(io_service);

	try
	{
		httpSocket->connect(*ep);
	}
	catch (std::exception& e)
	{
		std::cout << "IcecastStreamer::streamFile: couldn't connect to the server, retry..." << std::endl;
		httpSocket->lowest_layer().close();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		io_service.post([ep, fileName, this]() { streamFile(ep, fileName); });
		return;
	}

	if (!streamFile(httpSocket, fileName))
	{
		httpSocket->lowest_layer().close();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		io_service.post([ep, fileName, this]() { streamFile(ep, fileName); });
	}
};

template<typename T>
bool streamFileInner(std::shared_ptr<T> socket, const Uploading& uploading);

bool IcecastStreamer::streamFile(std::shared_ptr<boost::asio::ip::tcp::socket> httpSocket, const std::string& fileName)
{
	Uploading uploading;
	uploading.addres = addres;
	uploading.port = port;
	uploading.fileName = fileName;

	return streamFileInner(httpSocket, uploading);
}

template<typename T>
bool streamFileInner(std::shared_ptr<T> socket, const Uploading& uploading)
{
	WaveFile::WaveFileReader reader;
	
	if (!reader.open(uploading.fileName.c_str()))
	{
		std::cout << "Streamer: couldn't open the file to be streamed";
		return false;
	}

	const std::string NEWLINE = "\r\n";

	boost::asio::streambuf request;
	std::ostream request_stream(&request);

	boost::asio::streambuf response;

	{

		request_stream << "PUT /test HTTP/1.1" << NEWLINE;
		request_stream << "Host: " << uploading.addres << ":" << uploading.port << NEWLINE;
		request_stream << "User-Agent: IcecastTestStreamer" << NEWLINE;
		request_stream << "Transfer-Encoding: chunked" << NEWLINE;
		request_stream << "Content-Type: audio/vnd.wave" << NEWLINE;
		request_stream << "Expect: 100-continue" << NEWLINE;
		request_stream << "Authorization: Basic c291cmNlOnNvdXJjZV9wYXNzd29yZA==" << NEWLINE;

		request_stream << "Ice-Public: 1" << NEWLINE;
		request_stream << "Ice-Name: test_stream" << NEWLINE;
		request_stream << "Ice-Description: Hello, World!" << NEWLINE;

		request_stream << NEWLINE;

		try
		{
			socket->write_some(buffer(request.data()));

			read_until(*socket, response, "\r\n");

			std::string strResponse(boost::asio::buffer_cast<const char*>(response.data()), response.size());
			std::cout << strResponse << std::endl;

			if (strResponse.find("100 Continue") == std::string::npos)
			{
				return false;
			}
		}
		catch (std::exception &e)
		{
			std::cout << "UPLOADER: connection issues, retry..." << std::endl;
			return false;
		}
	}

	while (!reader.MusicFeof())
	{
		//const int BUFFER_SIZE = 102400;
		//unsigned char bufferX[BUFFER_SIZE];
		//reader.MusicFread(&bufferX[0], 1, BUFFER_SIZE);

		request.consume(request.size());
		request_stream << "HELLO" << std::endl;
		std::cout << request.size() << std::endl;
		std::cout << "HELLO" << std::endl;

		try
		{
			//socket->write_some(boost::asio::buffer(bufferX, BUFFER_SIZE));
			socket->write_some(buffer(request.data()));
		}
		catch (std::exception &e)
		{
			std::cout << "UPLOADER: connection issues, retry..." << std::endl;
			return false;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return true;
}