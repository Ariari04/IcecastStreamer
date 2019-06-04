#include "IcecastStreamer.h"

//#include <conio.h>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <wave/WaveFile.h>
#include <wave/WaveDecoder.h>
#include <mp3/Mp3Decoder.h>
#include <aac/AacDecoder.h>

#include <random>
#include <algorithm>
#include <iterator>
#include <iostream>


#include <id3/tag.h>


#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

#include <codecvt>
#include <string>

// convert UTF-8 string to wstring
std::wstring utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}

std::string url_encode(const std::string &value) {

	using namespace std;

	ostringstream escaped;
	escaped.fill('0');
	escaped << hex;

	for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		//if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
		if (c == '-' || c == '_' || c == '.' || c == '~' || (c >= '0' && c <= '9')) {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << uppercase;
		escaped << '%' << setw(2) << int((unsigned char)c);
		escaped << nouppercase;
	}

	return escaped.str();
}

struct ID3Metadata {

	std::string title;
	std::string artist;

};


unicode_t* ReverseEndian(unicode_t* pStr)
{
	for (int i = 0; pStr[i] != 0; ++i)
	{
		unicode_t tmp = (pStr[i] << 8) & 0xff00;
		tmp |= (pStr[i] >> 8) & 0x00ff;
		pStr[i] = tmp;
	}
	return pStr;
}

typedef unsigned short UTF16;

static size_t utf16strlen(const UTF16* szString)
{
	size_t len = 0;
	for (; szString[len] != 0; ++len);
	return len;
}


std::wstring ConvertUtf16ToKString(const UTF16* szString)
{
	if (sizeof(UTF16) == sizeof(wchar_t))
	{
		return std::wstring((const wchar_t*)szString);
	}
	else if (sizeof(UTF16) < sizeof(wchar_t))
	{
		const size_t len = utf16strlen(szString) + 1;
		std::vector<wchar_t> tmpStr(len);
		std::copy(szString, szString + len, tmpStr.begin());
		return std::wstring(&tmpStr.front());
	}
	return std::wstring();
}

std::wstring ConvertLatin1ToKString(const char* szString)
{
	size_t nSrcLen = strlen(szString);
	wchar_t* wszBuffer = new wchar_t[nSrcLen + 1];

	wszBuffer[nSrcLen] = 0;

	for (size_t i = 0; i < nSrcLen + 1; ++i)
		wszBuffer[i] = (unsigned char)szString[i];

	std::wstring result(wszBuffer);
	delete[] wszBuffer;
	return result;
}

std::wstring GetStringFromFrame(const ID3_Frame* pcFr, ID3_FieldID WantedId = ID3FN_TEXT)
{
	std::wstring res = L"";
	ID3_Frame::ConstIterator* it = pcFr->CreateIterator();
	const ID3_Field* f;
	while (NULL != (f = it->GetNext()))
	{
		ID3_TextEnc enc = f->GetEncoding();
		ID3_FieldID id = f->GetID();

		if (WantedId != id)
		{
			continue;
		}

		if (f->GetType() == ID3FTY_INTEGER)
		{
			//res.Format(_T("%d"), f->Get());
			res = boost::lexical_cast<std::wstring>(f->Get());
			break;
		}
		else
		{
			switch (enc)
			{
			case ID3TE_ISO8859_1:
			{
				const size_t nSize = f->Size() + 1;
				char* pStr = new char[nSize];
				f->Get(pStr, nSize);
				res += ConvertLatin1ToKString(pStr);
				delete[] pStr;
			}
			break;
			case ID3TE_UTF16:
			case ID3TE_UTF16BE:
			{
				const size_t nSizeInbytes = f->Size();
				// length of string in characters + trailing zero
				const size_t nLengthOfString = nSizeInbytes / sizeof(unicode_t) + 1;
				unicode_t* pStr = new unicode_t[nLengthOfString];
				f->Get(pStr, nLengthOfString);
				pStr[nLengthOfString - 1] = 0;
				if (enc == ID3TE_UTF16)
					pStr = ReverseEndian(pStr);
				res += ConvertUtf16ToKString(pStr);
				delete[] pStr;
			}
			break;
			}
		}
	}

	return res;
}

size_t getID3TagSize(const char* filename)
{
	ID3_Tag myTag(filename);
	
	return myTag.Size();
}

ID3Metadata getMetadata(const std::string& filename)
{
	ID3Metadata result{ "Unknown", "Unknown" };

	
	ID3_Tag myTag(filename.c_str());

	
	/*
	ID3_Tag::Iterator* iter = myTag.CreateIterator();
	ID3_Frame* myFrame = NULL;
	while (NULL != (myFrame = iter->GetNext()))
	{

		
		std::cout << myFrame->GetTextID() << std::endl;

		ID3_Frame::Iterator* iter2 = myFrame->CreateIterator();
		ID3_Field* myField = NULL;
		while (NULL != (myField = iter2->GetNext()))
		{
			std::cout << ">" << myField->GetType() << std::endl;

			if (myField->GetType() == 2)
			{

				std::wstring r = GetStringFromFrame(myFrame);

				std::cout << wstring_to_utf8(r) << std::endl;
				
			}
		}
		//delete iter2;
	}
	//delete iter;
	*/

	ID3_Frame* titleFrame = myTag.Find(ID3FID_TITLE);
	if (NULL != titleFrame)
	{
		std::wstring r = GetStringFromFrame(titleFrame);

		if (r != L"")
		{
			result.title = wstring_to_utf8(r);
		}
	}

	ID3_Frame* artistFrame = myTag.Find(ID3FID_BAND);
	if (NULL != artistFrame)
	{
		std::wstring r = GetStringFromFrame(artistFrame);

		if (r != L"")
		{
			//ID3FID_LEADARTIST
			result.artist = wstring_to_utf8(r);
		}
	}

	if (result.artist == "Unknown" || result.artist == "")
	{
		ID3_Frame* leadArtistFrame = myTag.Find(ID3FID_LEADARTIST);
		if (NULL != leadArtistFrame)
		{
			std::wstring r = GetStringFromFrame(leadArtistFrame);

			if (r != L"")
			{
				result.artist = wstring_to_utf8(r);
			}
		}
	}

	return result;
}




const long long MAX_UPLOADED_FILE_SIZE = 1024 * 1024 * 400;


IcecastStreamer::IcecastStreamer(boost::asio::io_service& ioService, std::string addres, std::string port)
	: io_service(ioService)
{
	this->addres = addres;
	this->port = port;
}


void IcecastStreamer::streamFile(const ContentToStream& contentToStream, std::shared_ptr<std::promise<void>> promise)
{
	std::cout << "Streamer streamFile 1" << std::endl;
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(addres, port);
	
	boost::system::error_code errcode;
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query, errcode);

	if (errcode)
	{
		std::cout << "IcecastStreamer::postUploadFileHttp: couldn't resolve addres, retry..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		io_service.post([contentToStream, promise, this]() { streamFile(contentToStream, promise); });
		return;
	}

	io_service.post([endpoint, contentToStream, promise, this]()
	{
		streamFile(endpoint, contentToStream, promise);
	});
}


//bool streamFileInner(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const Uploading& uploading);

void IcecastStreamer::streamFile(boost::asio::ip::tcp::endpoint endpoint, const ContentToStream& contentToStream, std::shared_ptr<std::promise<void>> promise)
{
	std::cout << "Streamer streamFile 2" << std::endl;
	std::shared_ptr<boost::asio::ip::tcp::socket> httpSocket = std::make_shared<boost::asio::ip::tcp::socket>(io_service);

	boost::system::error_code errcode;
	httpSocket->connect(endpoint, errcode);
	std::cout << "Streamer streamFile 2.1" << std::endl;
	if (errcode)
	{
		std::cout << "IcecastStreamer::streamFile: couldn't connect to the server, retry..." << std::endl;
		httpSocket->lowest_layer().close();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		io_service.post([endpoint, contentToStream, promise, this]() { streamFile(endpoint, contentToStream, promise); });
		return;
	}

	Uploading uploading;
	uploading.addres = addres;
	uploading.port = port;
	uploading.contentToStream = contentToStream;
	std::cout << "Streamer streamFile 2.2" << std::endl;
	if (!streamFileInner(httpSocket, uploading))
	{
		httpSocket->lowest_layer().close();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		io_service.post([endpoint, contentToStream, promise, this]() { streamFile(endpoint, contentToStream, promise); });
	}
	else
	{
		promise->set_value();
	}
}

std::shared_ptr<AudioDecoder> createReader(const std::string& fileName)
{
	auto ext = boost::filesystem::extension(fileName);

	std::shared_ptr<AudioDecoder> reader;

	IcecastStreamer::AudioFormat format = IcecastStreamer::AudioFormat::Invalid;
	if (ext == ".wav")
	{
		format = IcecastStreamer::AudioFormat::WAV;
		reader = std::make_shared<Decoding::WaveToMp3Decoder>();
	}
	else if (ext == ".mp3")
	{
		format = IcecastStreamer::AudioFormat::MP3;
		reader = std::make_shared<Decoding::Mp3WaveMp3Decoder>();
	}
	else if (ext == ".aac" || ext == ".m4a" || ext == ".mp4")
	{
		format = IcecastStreamer::AudioFormat::AAC;
		reader = std::make_shared<Decoding::AacToMp3Decoder>();
	}

	if (format == IcecastStreamer::AudioFormat::Invalid)
	{
		return nullptr;
	}

	if (!reader->open(fileName.c_str()))
	{
		std::cout << "IcecastStreamer: couldn't open the file to be streamed" << std::endl;
		return nullptr;
	}

	return reader;
}


bool streamOneReader(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::shared_ptr<AudioDecoder> reader)
{

	std::vector<char> Buffer;
	Buffer.resize(1024 * 1024);

	int packet = 0;

	int byteCount;

	while (true)
	{

		std::chrono::time_point<std::chrono::system_clock> nowBefore = std::chrono::system_clock::now();

		byteCount = reader->readDuration(&Buffer[0], Buffer.size(), std::chrono::seconds(3));

		if (byteCount < 1)
		{
			break;
		}

		auto asioBuffer = boost::asio::buffer(Buffer, byteCount);

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

		std::chrono::time_point<std::chrono::system_clock> nowAfter = std::chrono::system_clock::now();

		auto duration = std::chrono::seconds(3) - (nowAfter - nowBefore);

		std::this_thread::sleep_for(duration);
	}

	return true;
}

bool updateMetadata(boost::asio::io_service& io_service, const std::string& address, const std::string& port, const Uploading& uploading, const ID3Metadata& metadata)
{
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(address, port);

	boost::system::error_code errcode;
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query, errcode);

	std::shared_ptr<boost::asio::ip::tcp::socket> httpSocket = std::make_shared<boost::asio::ip::tcp::socket>(io_service);

	httpSocket->connect(endpoint, errcode);

	if (errcode)
	{
		std::cout << "IcecastStreamer::streamFile: couldn't connect to the server, retry..." << std::endl;
		httpSocket->lowest_layer().close();
		return false;
	}

	const std::string NEWLINE = "\r\n";

	boost::asio::streambuf request;
	std::ostream request_stream(&request);

	boost::asio::streambuf response;
	std::istream response_stream(&response);

	//std::string metadataString = metadata.artist + " - " + metadata.title;
	std::string metadataString = url_encode(metadata.artist + " - " + metadata.title);

	
	boost::erase_all(metadataString, "\n");
	boost::erase_all(metadataString, "\r");

	request_stream << "GET /admin/metadata?mount=/test.mp3&mode=updinfo&song=" + metadataString + " HTTP/1.1" << NEWLINE;
	request_stream << "Host: " << uploading.addres << ":" << uploading.port << NEWLINE;
	request_stream << "User-Agent: IcecastTestStreamer" << NEWLINE;
	request_stream << "Authorization: Basic YWRtaW46aGFja21l" << NEWLINE;

	request_stream << NEWLINE;

	try
	{
		httpSocket->send(buffer(request.data(), request.size()));

		int byteCount = boost::asio::read_until(*httpSocket, response, '\r') - 1;

		std::string responseCode(byteCount, ' ');
		response_stream.read(&responseCode[0], byteCount);

		std::cout << "Icecast Server Response: " << responseCode << std::endl;

		if (responseCode.find("200 OK") == std::string::npos)
		{
			return false;
		}
	}
	catch (std::exception &e)
	{
		std::cout << "IcecastStreamer: connection issues, retry..." << std::endl;
		return false;
	}

	return true;
}



bool IcecastStreamer::streamFileInner(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const Uploading& uploading)
{
	const std::string NEWLINE = "\r\n";

	boost::asio::streambuf request;
	std::ostream request_stream(&request);

	boost::asio::streambuf response;
	std::istream response_stream(&response);

	
		request_stream << "PUT /test.mp3 HTTP/1.1" << NEWLINE;
		request_stream << "Host: " << uploading.addres << ":" << uploading.port << NEWLINE;
		request_stream << "User-Agent: IcecastTestStreamer" << NEWLINE;
		request_stream << "Transfer-Encoding: chunked" << NEWLINE;
		request_stream << "Content-Type: audio/mpeg" << NEWLINE;
		//request_stream << "Content-Type: audio/vnd.wave" << NEWLINE;
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
	

	while (true)
	{
		std::random_device rd;
		std::mt19937 g(rd());

#ifdef _WIN32
		auto shuffledPlaylist = uploading.contentToStream.playlist;
		

		
		std::shuffle(shuffledPlaylist.begin(), shuffledPlaylist.end(), g);
		
		for (size_t i = 0; i < shuffledPlaylist.size(); i++)
		{
			std::string prefix = "D:/music/";
			std::string fileName = prefix + shuffledPlaylist[i];
			auto reader = createReader(fileName);

			ID3Metadata metadata = getMetadata(fileName);

			updateMetadata(io_service, this->addres, this->port, uploading, metadata);

			if (!reader)
			{
				std::cout << "File can't be played: " << shuffledPlaylist[i] << std::endl;
			}
			else
			{
				std::cout << "Start playing: " << shuffledPlaylist[i] << std::endl;

				if (streamOneReader(socket, reader))
				{
					std::cout << "File played successfully: " << shuffledPlaylist[i] << std::endl;
				}
				else
				{
					std::cout << "Error when playing file: " << shuffledPlaylist[i] << std::endl;
				}
			}
		}
#else

		//std::cout << "Download playlist before" << std::endl;
		//auto playlist = downloadPlaylist();
		auto playlist = loadPlaylistFromFile();
		//std::cout << "Download playlist after" << std::endl;

		//auto playlist = uploading.contentToStream.playlist;

		std::cout << "Playlist size: " << playlist.size() << std::endl;

		std::uniform_int_distribution<> dis(0, playlist.size()-1);

		size_t i = dis(g);


		//std::string prefix = "/home/ubuntu/";
		std::string prefix = "/home/ubuntu";
		std::string fileName = prefix + playlist[i];
		std::cout << "Filename is: " << fileName << std::endl;
		auto reader = createReader(fileName);

		ID3Metadata metadata = getMetadata(fileName);

		updateMetadata(io_service, this->addres, this->port, uploading, metadata);

		if (!reader)
		{
			std::cout << "File can't be played: " << playlist[i] << std::endl;
		}
		else
		{
			std::cout << "Start playing: " << playlist[i] << std::endl;

			if (streamOneReader(socket, reader))
			{
				std::cout << "File played successfully: " << playlist[i] << std::endl;
			}
			else
			{
				std::cout << "Error when playing file: " << playlist[i] << std::endl;
			}
		}

#endif
	}

	std::cout << "IcecastStreamer: stream is finished" << std::endl;

	return true;
}


std::vector<std::string> IcecastStreamer::loadPlaylistFromFile()
{
	std::ifstream t("/home/ubuntu/playlist.txt");
	std::string playlistString((std::istreambuf_iterator<char>(t)),
					 std::istreambuf_iterator<char>());
				 
	boost::trim(playlistString);

	std::vector<std::string> strs;
	boost::split(strs, playlistString, boost::is_any_of(" \n"));
	
	return strs;
}


std::vector<std::string> IcecastStreamer::downloadPlaylist()
{
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query("528records.com", "80");
	//boost::asio::ip::tcp::resolver::query query("id3lib.sourceforge.net", "80");

	boost::system::error_code errcode;
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query, errcode);

	std::shared_ptr<boost::asio::ip::tcp::socket> httpSocket = std::make_shared<boost::asio::ip::tcp::socket>(io_service);

	httpSocket->connect(endpoint, errcode);

	if (errcode)
	{
		std::cout << "IcecastStreamer::downloadPlaylist: couldn't connect to the server" << std::endl;
		httpSocket->lowest_layer().close();
		return {};
	}

	const std::string NEWLINE = "\r\n";

	boost::asio::streambuf request;
	std::ostream request_stream(&request);

	boost::asio::streambuf response;
	std::istream response_stream(&response);

	//boost::asio::streambuf response2;
	//std::istream response_stream2(&response2);

	//std::string metadataString = metadata.artist + " - " + metadata.title;
	//std::string metadataString = url_encode(metadata.artist + " - " + metadata.title);


	//boost::erase_all(metadataString, "\n");
	//boost::erase_all(metadataString, "\r");

	request_stream << "GET /radio/api/qjuvfzlpcjmfvful11fk/main/9 HTTP/1.1" << NEWLINE;
	//request_stream << "GET / HTTP/1.1" << NEWLINE;
	//request_stream << "Host: id3lib.sourceforge.net:80" << NEWLINE;
	request_stream << "Host: 528records.com:80" << NEWLINE;
	request_stream << "User-Agent: IcecastTestStreamer" << NEWLINE;

	request_stream << NEWLINE;

	int byteCount = 0;

	std::array<char, 1024> charbuf;

	std::string playlistString;

	try
	{
		httpSocket->send(buffer(request.data(), request.size()));

		/*
		byteCount = boost::asio::read_until(*httpSocket, response, '\r');

		//playlistString += std::string(charbuf.begin(), charbuf.begin() + byteCount);

		playlistString.resize(byteCount, ' ');
		//std::string responseCode(byteCount, ' ');
		response_stream.read(&playlistString[0], byteCount);

		std::cout << "Icecast Server Response: " << playlistString << std::endl;

		if (playlistString.find("200 OK") == std::string::npos)
		{
			return {};
		}
		*/
		boost::system::error_code c;

		byteCount = 0;

		do
		{
			byteCount = boost::asio::read(*httpSocket, boost::asio::buffer(charbuf), c);
			playlistString += std::string(charbuf.begin(), charbuf.begin() + byteCount);
		} while (!c);
	}
	catch (std::exception &e)
	{
		std::cout << "IcecastStreamer: connection issues" << std::endl;
		return {};
	}

	std::string httpOk = "HTTP/1.1 200 OK\r\n";

	std::cout << "Playlist string: >>>" << std::endl;
	std::cout << playlistString << std::endl;

	if (playlistString.substr(0, httpOk.size()) != httpOk)
	{
		return {};
	}
	
	
	std::size_t found = playlistString.find("\r\n\r\n");

	if (found == std::string::npos)
	{
		return {};
	}


	playlistString.erase(playlistString.begin(), playlistString.begin() + found + 4);

	boost::trim(playlistString);

	std::vector<std::string> strs;
	boost::split(strs, playlistString, boost::is_any_of(" "));


	return strs;
}
