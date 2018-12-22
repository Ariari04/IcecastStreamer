#include "IcecastStreamer.h"

#include <conio.h>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <wave/WaveFile.h>
#include <wave/WaveDecoder.h>
#include <wave/WaveEncoder.h>
#include <mp3/Mp3Decoder.h>
#include <mp3/Mp3Encoder.h>

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
}

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

	auto ext = boost::filesystem::extension(uploading.fileName);

	std::shared_ptr<AudioDecoder> reader;

	IcecastStreamer::AudioFormat format = IcecastStreamer::AudioFormat::Invalid;
	if (ext == ".wav")
	{
		format = IcecastStreamer::AudioFormat::WAV;
		reader = std::make_shared<Decoding::WaveDecoder>();
	}
	else if (ext == ".mp3")
	{
		format = IcecastStreamer::AudioFormat::MP3;
		reader = std::make_shared<Decoding::Mp3Decoder>();
	}

	if (format == IcecastStreamer::AudioFormat::Invalid)
	{
		return false;
	}

	if (!reader->open(uploading.fileName.c_str()))
	{
		std::cout << "IcecastStreamer: couldn't open the file to be streamed";
		return false;
	}

	std::cout << "IcecastStreamer: press any key to start streaming";
	_getch();

	char Buffer[2 * 1152 * 2];

	std::ofstream ofs("test/fff.wav", std::ios::binary);

	int packet = 0;

	while (true)
	{
		int byteCount = reader->read(Buffer, 2 * 1152 * 2);

		if (byteCount < 1)
		{
			break;
		}

		auto asioBuffer = boost::asio::buffer(Buffer, byteCount);

		ofs.write((char*)Buffer, byteCount);

		try
		{
			socket->send(asioBuffer);
			std::cout << "IcecastStreamer: streaming... " << ++packet << " : " << byteCount << std::endl;
		}
		catch (std::exception &e)
		{
			std::cout << "IcecastStreamer: connection issues, retry..." << std::endl;
			return false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}

	std::cout << "IcecastStreamer: stream is finished" << std::endl;

	return true;
}

void IcecastStreamer::saveSound(const std::string& binarySoundFile, const std::string& savedFile)
{
	char binarySound[2 * 1152 * 2]; // MP3 chunks have size 2 * 1152 * 2, WAVE chunks may be different, let's take 2 * 1152 * 2 as the size of the buffer

	std::shared_ptr<AudioEncoder> writer;

	auto ext = boost::filesystem::extension(savedFile);
	IcecastStreamer::AudioFormat format = IcecastStreamer::AudioFormat::Invalid;
	if (ext == ".wav")
	{
		format = IcecastStreamer::AudioFormat::WAV;
		writer = std::make_shared<Encoding::WaveEncoder>();
	}
	else if (ext == ".mp3")
	{
		format = IcecastStreamer::AudioFormat::MP3;
		writer = std::make_shared<Encoding::Mp3Encoder>();
	}

	writer->open(savedFile.c_str());

	std::ifstream raw{ binarySoundFile, std::ios::binary };
	do
	{
		raw.read((char*)binarySound, 2 * 1152 * 2);
		int count = raw.gcount();

		writer->write(binarySound, count);
		if (count < 1)
		{
			break;
		}
	}
	while (true);
}