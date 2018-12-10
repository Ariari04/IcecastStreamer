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


void IcecastStreamer::streamFile(const std::string& fileName, std::shared_ptr<std::promise<void>> promise)
{
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(addres, port);
	
	boost::system::error_code errcode;
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query, errcode);

	if (errcode)
	{
		std::cout << "IcecastStreamer::postUploadFileHttp: couldn't resolve addres, retry..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		io_service.post([fileName, promise, this]() { streamFile(fileName, promise); });
		return;
	}

	io_service.post([endpoint, fileName, promise, this]()
	{
		streamFile(endpoint, fileName, promise);
	});
}

template<typename T>
bool streamFileInner(std::shared_ptr<T> socket, const Uploading& uploading);

void IcecastStreamer::streamFile(boost::asio::ip::tcp::endpoint endpoint, const std::string& fileName, std::shared_ptr<std::promise<void>> promise)
{
	std::shared_ptr<boost::asio::ip::tcp::socket> httpSocket = std::make_shared<boost::asio::ip::tcp::socket>(io_service);

	boost::system::error_code errcode;
	httpSocket->connect(endpoint, errcode);
	
	if (errcode)
	{
		std::cout << "IcecastStreamer::streamFile: couldn't connect to the server, retry..." << std::endl;
		httpSocket->lowest_layer().close();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		io_service.post([endpoint, fileName, promise, this]() { streamFile(endpoint, fileName, promise); });
		return;
	}

	Uploading uploading;
	uploading.addres = addres;
	uploading.port = port;
	uploading.fileName = fileName;

	if (!streamFileInner(httpSocket, uploading))
	{
		httpSocket->lowest_layer().close();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		io_service.post([endpoint, fileName, promise, this]() { streamFile(endpoint, fileName, promise); });
	}
	else
	{
		promise->set_value();
	}
};

template<typename T>
bool streamFileInner(std::shared_ptr<T> socket, const Uploading& uploading)
{
	const std::string NEWLINE = "\r\n";

	boost::asio::streambuf request;
	std::ostream request_stream(&request);

	boost::asio::streambuf response;
	std::istream response_stream(&response);

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
			socket->send(buffer(request.data(), request.size()));

			int byteCount = boost::asio::read_until(*socket, response, '\r') - 1;

			std::string responseCode(byteCount, ' ');
			response_stream.read(&responseCode[0], byteCount);

			std::cout << "Icecast Server Response: " << responseCode << std::endl;

			if (responseCode.find("100 Continue") == std::string::npos)
			{
				return false;
			}
		}
		catch (std::exception &e)
		{
			std::cout << "IcecastStreamer: connection issues, retry..." << std::endl;
			return false;
		}
	}

	const int BUFFER_SIZE = 102400;
	char dataBuffer[BUFFER_SIZE];

	WaveFile::WaveFileReader reader;

	if (!reader.open(uploading.fileName.c_str()))
	{
		std::cout << "IcecastStreamer: couldn't open the file to be streamed";
		return false;
	}

	for (int i = 0; i < 5; ++i)
	{
		std::cout << "IcecastStreamer: start stream in " << 5 - i << " seconds" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	while (!reader.MusicFeof())
	{
		int byteCount = reader.MusicFread(dataBuffer, 1, BUFFER_SIZE);
		auto asioBuffer = boost::asio::buffer(dataBuffer, byteCount);

		try
		{
			socket->send(asioBuffer);
			std::cout << "IcecastStreamer: streaming..." << std::endl;
		}
		catch (std::exception &e)
		{
			std::cout << "IcecastStreamer: connection issues, retry..." << std::endl;
			return false;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	std::cout << "IcecastStreamer: stream is finished" << std::endl;

	return true;
}

void IcecastStreamer::saveWaveSound(const std::string& binarySoundFile, const std::string& waveFile)
{
	const int BUFFER_SIZE = 102400;
	char binarySound[BUFFER_SIZE];

	std::ifstream ifs{ binarySoundFile, std::ios::binary };

	WaveFile::WaveFileWriter writer(waveFile.c_str());
	writer.writeHeader();

	while (!ifs.eof())
	{
		ifs.read(binarySound, BUFFER_SIZE);
		int byteCount = ifs.gcount();
		writer.MusicFwrite(binarySound, 1, byteCount);
	}
}