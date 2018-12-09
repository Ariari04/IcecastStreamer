// Conversion528.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "WaveFile.h"
#include "Conversion.h"
#include <memory.h>
#include "WaveFileAdapter.h"

#include "Converter528.h"
#include "FlatConverter.h"
#include "ExperimentalConverter.h"

#define _WRITE_ERROR_DESCR_
#ifdef _WRITE_ERROR_DESCR_

#include <fstream>
#include "ConsoleHelpers.h"

#endif

#include "boost/filesystem.hpp"
#include <boost/thread/thread.hpp>
#include "boost/asio.hpp"

#include <server/IcecastStreamer.h>

boost::asio::io_service ioService;
boost::asio::io_service::work work(ioService);
boost::thread_group serverThreadPool;

IcecastStreamer streamer{ ioService, "127.0.0.1", "8000" };

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
	ioService.post([fileName]()
	{
		streamer.streamFile(fileName);
	});
}

void ProcessFileName(const std::string& fileName)
{
	streamFile(fileName);
}

int main(int argc, char* argv[])
{
	
	//const char* fileName = "D:\\Work\\TorchProjects\\converter\\Conversion528\\Track5.wav";

	//const char* fileName = "D:\\Work\\TorchProjects\\converter\\Conversion528\\usp_unsil-1.wav";
	//const char* fileName = "D:\\Work\\TorchProjects\\converter\\Conversion528\\vip.wav";
	//const char* fileName = "D:\\Work\\TorchProjects\\converter\\Conversion528\\tone.wav";
	//const char* fileName = "test/record.wav";

	//http::server::server server(ioService, "127.0.0.1", "8000");
	auto f = []() {
		ioService.run();
	};
	serverThreadPool.create_thread(f);

	ProcessFileName("test/record.wav");

	std::this_thread::sleep_for(std::chrono::seconds(100000));

	ioService.stop();
	serverThreadPool.join_all();

	return 0;
}

