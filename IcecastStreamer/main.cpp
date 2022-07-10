// Conversion528.cpp : Defines the entry point for the console application.
//

#ifdef _WIN32
#include <conio.h>

#include "stdafx.h"
#endif
#include "wave/WaveFile.h"

#include <memory.h>

#define _WRITE_ERROR_DESCR_
#ifdef _WRITE_ERROR_DESCR_

#include <fstream>
#endif

#include "boost/filesystem.hpp"
#include <boost/thread/thread.hpp>
#include "boost/asio.hpp"


#include <icecast/IcecastStreamer.h>

#include <random>
#include <algorithm>
#include <iterator>
#include <iostream>


boost::asio::io_service ioService;
boost::asio::io_service::work work(ioService);
boost::thread_group threadPool;

//IcecastStreamer streamer{ ioService, "vm493.vmware.nano.lv", "80" };
#ifdef _WIN32
IcecastStreamer streamer{ ioService, "127.0.0.1", "80" };

#else
//IcecastStreamer streamer{ ioService, "528records.com", "8000" };
IcecastStreamer streamer{ ioService, "127.0.0.1", "80" };
#endif
//IcecastStreamer streamer{ ioService, "528records.com", "8000" };
//IcecastStreamer streamer{ ioService, "127.0.0.1", "80" };

std::vector<std::string> getFileNamesInFolder(const std::string& folder)
{
	using namespace std;
	using namespace boost::filesystem;

	path p(folder);

	std::vector<std::string> result;

	directory_iterator end_itr;

	// cycle through the directory
	for (directory_iterator itr(p); itr != end_itr; ++itr)
	{
		// If it's not a directory, list it. If you want to list directories too, just remove this check.
		if (is_regular_file(itr->path())) {
			// assign current file name to current_file and echo it out to the console.
			string current_file = itr->path().string();
			
			result.push_back(current_file);
		}
	}

	return result;
}

void streamFile(const std::string& fileName)
{
	auto promise = std::make_shared<std::promise<void>>();

	ContentToStream contentToStream(fileName);

	ioService.post([contentToStream, promise]()
	{
		streamer.streamFile(contentToStream, promise);
	});

	promise->get_future().wait();
}

void streamPlaylist(const std::vector<std::string>& playlist)
{
	auto promise = std::make_shared<std::promise<void>>();

	ContentToStream contentToStream(playlist);

	ioService.post([contentToStream, promise]()
	{
		std::cout << "StreamPlaylist inner 1" << std::endl;
		//streamer.streamFile(contentToStream, promise);

		streamer.streamFileLooped(contentToStream, promise);
	});

	promise->get_future().wait();
}

/*int main(int argc, char* argv[])
{
#ifdef _WIN32
	std::cout << "Press any key to start playing:" << std::endl;
	_getwch();
#endif



	return 0;
}
*/

int main(int argc, char* argv[])
{
	auto f = []()
	{
		ioService.run();
	};

	threadPool.create_thread(f);


	//std::vector<std::string> listOfFiles = { "168446101.aac", "b5c751d94e8b[1].mp3" };

	//std::vector<std::string> listOfFiles = { "b5c751d94e8b[1].mp3" };

	//std::vector<std::string> listOfFiles = { "everlasting_summer_03.mp3", "bala.mp3" };

	//std::vector<std::string> listOfFiles = { "FLASHBACK FM - GTA 3.wav" };

	//std::vector<std::string> listOfFiles = { "GTA Vice City - Flash FM.mp3" };

	//std::vector<std::string> listOfFiles = { "E:/music/BAAM.wav", "E:/music/bala.wav"};
	//std::vector<std::string> listOfFiles = { "E:/music/BAAM.wav" };

	std::vector<std::string> listOfFiles = { "E:/music/death note.ogg" };
	
	
	
	std::cout << "Streamed created" << std::endl;

#ifdef _WIN32
	std::cout << "Press any key to start playing:" << std::endl;
	_getwch();
#endif

	do
	{
		streamPlaylist(listOfFiles);


	} while (true);
	
	ioService.stop();
	threadPool.join_all();

	return 0;
}


