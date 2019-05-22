// Conversion528.cpp : Defines the entry point for the console application.
//

#include <conio.h>
#include "stdafx.h"
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
		streamer.streamFile(contentToStream, promise);
	});

	promise->get_future().wait();
}


void StreamFile()
{
	std::cout << "Choose a file:" << std::endl;
	std::cout << "1) Special" << std::endl;
	std::cout << "2) test/original.mp3" << std::endl;
	std::cout << "3) test/original.aac" << std::endl;
	std::cout << "4) test/original.m4a" << std::endl;
	std::cout << "5) test/original.mp4" << std::endl;

	char key = _getwch();

	switch (key)
	{
	case '1': streamFile("D:/music/BAAM.wav"); break;
	//case '1': streamFile("D:/music/GTA Vice City - Flash FM.mp3"); break;
	//case '1': streamFile("D:/music/PRISTIN V - Get It.mp3"); break;
	//case '1': streamFile("D:/music/168446101.aac"); break;
	case '2': streamFile("test/original.mp3"); break;
	case '3': streamFile("test/original.aac"); break;
	case '4': streamFile("test/original.m4a"); break;
	case '5': streamFile("test/original.mp4"); break;
	}
}

int main(int argc, char* argv[])
{
	auto f = []()
	{
		ioService.run();
	};

	threadPool.create_thread(f);

	//std::vector<std::string> listOfFiles = { "BAAM.wav", "PRISTIN V - Get It.mp3", "168446101.aac", "not_existing_file.mp3" };

	std::vector<std::string> listOfFiles = { "PRISTIN V - Get It.mp3" };

	auto playlist = listOfFiles;

	std::cout << "Press any key to start playing:" << std::endl;

	_getwch();

	do
	{
		streamPlaylist(listOfFiles);


	} while (true);
	/*
	char key;
	do
	{
		std::cout << "Choose an action:" << std::endl;
		std::cout << "1) stream raw PCM sound from a file" << std::endl;

		key = _getwch();

		switch (key)
		{
		case '1':
			StreamFile();
			break;
		}

	} while (true);*/

	ioService.stop();
	threadPool.join_all();

	return 0;
}

