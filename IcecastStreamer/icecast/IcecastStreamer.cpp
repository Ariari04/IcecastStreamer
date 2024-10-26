#include "IcecastStreamer.h"

//#include <conio.h>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "../wave/WaveFile.h"
#include "../wave/WaveDecoder.h"
#include "../mp3/Mp3DecoderNew.h"
#include "../aac/AacDecoder.h"

#include <random>
#include <algorithm>
#include <iterator>
#include <iostream>


//#include <id3/tag.h>


#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

#include <codecvt>
#include <string>


/*
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

std::string url_encode(const std::string& value) {

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
}*/




const long long MAX_UPLOADED_FILE_SIZE = 1024 * 1024 * 400;


IcecastStreamer::IcecastStreamer(boost::asio::io_service& ioService, std::string addres, std::string port, std::string password)
	: io_service(ioService)
{
	this->addres = addres;
	this->port = port;
	this->password = password;
}






/*
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

	//request_stream << "GET /admin/metadata?mount=/test.mp3&mode=updinfo&song=" + metadataString + " HTTP/1.1" << NEWLINE;
	request_stream << "GET /admin/metadata?mount=/main_station_premium&mode=updinfo&song=" + metadataString + " HTTP/1.1" << NEWLINE;
	request_stream << "Host: " << uploading.addres << ":" << uploading.port << NEWLINE;
	request_stream << "User-Agent: IcecastTestStreamer" << NEWLINE;
	//request_stream << "Authorization: Basic YWRtaW46aGFja21l" << NEWLINE;
	request_stream << "Authorization: Basic YWRtaW46Z3lFTUhDcDI5SGtVb1pyYmt4cWc=" << NEWLINE;

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
	catch (std::exception& e)
	{
		std::cout << "IcecastStreamer: connection issues, retry..." << std::endl;
		return false;
	}

	return true;
}
*/

void IcecastStreamer::streamFileLooped(const ContentToStream& contentToStream, std::shared_ptr<std::promise<void>> promise)
{
	std::cout << "Streamer streamFileLooped 1" << std::endl;
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(addres, port);

	boost::system::error_code errcode;
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query, errcode);

	if (errcode)
	{
		std::cout << "IcecastStreamer::postUploadFileHttp: couldn't resolve addres, retry..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		io_service.post([contentToStream, promise, this]() { streamFileLooped(contentToStream, promise); });
		return;
	}

	io_service.post([endpoint, contentToStream, promise, this]()
		{
			streamFileLooped(endpoint, contentToStream, promise);
		});
}

void IcecastStreamer::streamFileLooped(boost::asio::ip::tcp::endpoint endpoint, const ContentToStream& contentToStream, std::shared_ptr<std::promise<void>> promise)
{
	std::cout << "Streamer streamFileLooped 2" << std::endl;
	std::shared_ptr<boost::asio::ip::tcp::socket> httpSocket = std::make_shared<boost::asio::ip::tcp::socket>(io_service);

	boost::system::error_code errcode;
	httpSocket->connect(endpoint, errcode);
	std::cout << "Streamer streamFileLooped 2.1" << std::endl;
	if (errcode)
	{
		std::cout << "IcecastStreamer::streamFileLooped: couldn't connect to the server, retry..." << std::endl;
		httpSocket->lowest_layer().close();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		io_service.post([endpoint, contentToStream, promise, this]() { streamFileLooped(endpoint, contentToStream, promise); });
		return;
	}

	Uploading uploading;
	uploading.addres = addres;
	uploading.port = port;
	uploading.contentToStream = contentToStream;
	std::cout << "Streamer streamFileLooped 2.2" << std::endl;

	if (!streamFileLoopedInner(httpSocket, uploading))
	{
		httpSocket->lowest_layer().close();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		io_service.post([endpoint, contentToStream, promise, this]() { streamFileLooped(endpoint, contentToStream, promise); });
	}
	else
	{
		promise->set_value();
	}
}

std::shared_ptr<AudioDecoderInterface> selectReaderByFileName(const std::string fileName)
{
	std::shared_ptr<AudioDecoderInterface> reader;

	size_t fileNameSize = fileName.size();

	if (fileName[fileNameSize - 3] == 'm' && fileName[fileNameSize - 2] == 'p' && fileName[fileNameSize - 1] == '3')
	{
		reader = std::make_shared<DecodingX::Mp3WaveMp3DecoderNew>();
	}
	else if (fileName[fileNameSize - 3] == 'o' && fileName[fileNameSize - 2] == 'g' && fileName[fileNameSize - 1] == 'g')
	{
		reader = std::make_shared<Decoding::OggDecoder>();
	}
	else if (fileName[fileNameSize - 3] == 'w' && fileName[fileNameSize - 2] == 'a' && fileName[fileNameSize - 1] == 'v')
	{
		reader = std::make_shared<Decoding::WaveDecoder>();
	}
	else if (fileName[fileNameSize - 3] == 'a' && fileName[fileNameSize - 2] == 'a' && fileName[fileNameSize - 1] == 'c')
	{
		reader = std::make_shared<Decoding::AacDecoder>();
	}
	else
	{
		throw std::runtime_error("Unknown extension");
	}

	return reader;
}

std::string base64_encode(const std::string& input) {
	const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	std::string encoded;
	int val = 0, valb = -6;
	for (unsigned char c : input) {
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0) {
			encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6) {
		encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
	}
	while (encoded.size() % 4) {
		encoded.push_back('=');
	}
	return encoded;
}


bool IcecastStreamer::streamFileLoopedInner(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const Uploading& uploading)
{
	const std::string NEWLINE = "\r\n";

	boost::asio::streambuf request;
	std::ostream request_stream(&request);

	boost::asio::streambuf response;
	std::istream response_stream(&response);

	

	request_stream << "PUT /output HTTP/1.1" << NEWLINE;
	//request_stream << "PUT /main_station_premium HTTP/1.1" << NEWLINE;
	request_stream << "Host: " << uploading.addres << ":" << uploading.port << NEWLINE;
	request_stream << "User-Agent: IcecastTestStreamer" << NEWLINE;
	request_stream << "Transfer-Encoding: chunked" << NEWLINE;
	//request_stream << "Content-Type: audio/mpeg" << NEWLINE;
	request_stream << "Content-Type: audio/ogg" << NEWLINE;
	//request_stream << "Content-Type: audio/vnd.wave" << NEWLINE;
	request_stream << "Expect: 100-continue" << NEWLINE;
#ifdef _WIN32
	// source:abcde123
	request_stream << "Authorization: Basic " + base64_encode("source:" + password) << NEWLINE;
#else
	//request_stream << "Authorization: Basic c291cmNlOkQ0a3UyUVRTR1pUbmJOQjhUMVU3" << NEWLINE;
	request_stream << "Authorization: Basic c291cmNlOnNvdXJjZV9wYXNzd29yZA==" << NEWLINE;
#endif
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
	catch (std::exception& e)
	{
		std::cout << "IcecastStreamer: connection issues, retry..." << std::endl;
		return false;
	}


	auto playlist = uploading.contentToStream.playlist;

	
	bool shuffle = true;

	bool repeat = true;

	if (playlist.size() == 0)
	{
		throw std::runtime_error("Playlist is empty");
	}

	if (shuffle)
	{
		std::random_device rd;
		std::mt19937 g(rd());

		auto shuffledPlaylist = uploading.contentToStream.playlist;

		std::shuffle(shuffledPlaylist.begin(), shuffledPlaylist.end(), g);

		playlist = shuffledPlaylist;
	}

	size_t playlistIndex = 0;

	size_t nextPlaylistIndex = (playlistIndex + 1) % playlist.size();

	std::string& currentTrack = playlist[playlistIndex];

	std::string& nextTrack = playlist[nextPlaylistIndex];

	std::shared_ptr<AudioDecoderInterface> reader = selectReaderByFileName(currentTrack);

	//std::shared_ptr<Decoding::WaveDecoder> reader = std::make_shared<Decoding::WaveDecoder>();
	//std::shared_ptr<AudioDecoderInterface> reader = std::make_shared<Decoding::OggDecoder>();
	//std::shared_ptr<AudioDecoderInterface> reader = std::make_shared<DecodingX::Mp3WaveMp3DecoderNew>();
	//std::shared_ptr<AudioDecoderInterface> reader = std::make_shared<Decoding::AacDecoder>();

	reader->open(currentTrack.c_str());

	//std::shared_ptr<AudioDecoderInterface> secondReader = std::make_shared<Decoding::OggDecoder>();
	//std::shared_ptr<AudioDecoderInterface> secondReader = std::make_shared<DecodingX::Mp3WaveMp3DecoderNew>();
	std::shared_ptr<AudioDecoderInterface> secondReader = selectReaderByFileName(nextTrack);

	secondReader->open(nextTrack.c_str());

	std::unique_ptr<Decoding::WavToOggConverter> writer = std::make_unique<Decoding::WavToOggConverter>();

	writer->openOutput();

	static std::array<char, 2 * 1024 * 1024> IntermediateBuffer; //Must fit 174,000
	static std::array<char, 64 * 1024> Buffer;

	int packet = 0;

	int byteCount = 0;
	int readByteCount = 0;


	constexpr auto defaultDuration = std::chrono::milliseconds(1000);

	bool keepGoing = true;


	while (keepGoing)
	{
		//byteCount = 0;

		std::chrono::time_point<std::chrono::system_clock> nowBefore = std::chrono::system_clock::now();

		std::chrono::milliseconds actualDurationRead;

		readByteCount = reader->readDuration(&IntermediateBuffer[0], IntermediateBuffer.size(), defaultDuration, actualDurationRead);


		if (actualDurationRead < defaultDuration)
		{

			if ((nextPlaylistIndex == 0) && (!repeat))
			{
				//Do nothing...
			}
			else
			{

				//playlistIndex += 1; //We don't need this
				nextPlaylistIndex += 1;


				//playlistIndex = playlistIndex % playlist.size(); //We don't need this
				nextPlaylistIndex = nextPlaylistIndex % playlist.size();

				reader = secondReader;
				secondReader = selectReaderByFileName(playlist[nextPlaylistIndex]);


				secondReader->open(playlist[nextPlaylistIndex].c_str());

				//Show must go on
				//byteCount = writer->convertData(&IntermediateBuffer[0], readByteCount, &Buffer[0], Buffer.size());

				//We assume that each music file is actually longer than defaultDuration
				std::chrono::milliseconds secondActualDurationRead;
				readByteCount += reader->readDuration(&IntermediateBuffer[readByteCount], IntermediateBuffer.size() - readByteCount, defaultDuration, secondActualDurationRead);

				actualDurationRead += secondActualDurationRead;
			}
		}


		if (readByteCount == 0)
		{
			//If this happens, we are at the end of the file
			//If we loop, this should not happen at all
			byteCount = writer->finishConvertData(&Buffer[0], Buffer.size());
			keepGoing = false;
		}
		else
		{
			byteCount = writer->convertData(&IntermediateBuffer[0], readByteCount, &Buffer[0], Buffer.size());
		}

		auto asioBuffer = boost::asio::buffer(Buffer, byteCount);

		try
		{
			socket->send(asioBuffer);
			std::cout << "IcecastStreamer: streaming... " << ++packet << " : " << byteCount << std::endl;
		}
		catch (std::exception& e)
		{
			std::cout << "IcecastStreamer: connection issues, retry..." << std::endl;
			return false;
		}

		std::chrono::time_point<std::chrono::system_clock> nowAfter = std::chrono::system_clock::now();

		auto duration = actualDurationRead - (nowAfter - nowBefore);

		std::this_thread::sleep_for(duration);
	}

	std::cout << "IcecastStreamer: stream is finished" << std::endl;

	return true;
}


/*
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

		boost::system::error_code c;

		byteCount = 0;

		do
		{
			byteCount = boost::asio::read(*httpSocket, boost::asio::buffer(charbuf), c);
			playlistString += std::string(charbuf.begin(), charbuf.begin() + byteCount);
		} while (!c);
	}
	catch (std::exception& e)
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
*/
