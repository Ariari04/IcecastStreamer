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
IcecastStreamer streamer{ ioService, "528records.com", "8000" };

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
		streamer.streamFile(contentToStream, promise);
	});

	promise->get_future().wait();
}

int main(int argc, char* argv[])
{
	auto f = []()
	{
		ioService.run();
	};

	threadPool.create_thread(f);


	//std::vector<std::string> listOfFiles = { "BAAM.wav", "PRISTIN V - Get It.mp3", "168446101.aac", "not_existing_file.mp3" };

	std::vector<std::string> listOfFiles = { "b5c751d94e8b[1].mp3" };
	
/*
	std::vector<std::string> listOfFiles = { "K-pop old/(CNBLUE) - Black Flower.mp3",
		"K-pop old/(CNBLUE) - LOVE GIRL.mp3",
		"K-pop old/09. Day By Day (데이바이데이) (Japanese ver.).mp3",
		"K-pop old/10. TTL (Time To Love).mp3",
		"K-pop old/11. Cry Cry (크라이 크라이) (Japanese Version).mp3",
		"K-pop old/11. Cry Cry (크라이 크라이).mp3",
		"K-pop old/13. Day By Day (데이바이데이).mp3",
		"K-pop old/AOA - Good Luck.mp3",
		"K-pop old/CNBLUE - Blind love [korean version].mp3",
		"K-pop old/CNBLUE [    ] - (I'm a loner).mp3",
		"K-pop old/f(x) - Beautiful Stranger.mp3",
		"K-pop old/INFINITE - Paradise.mp3",
		"K-pop old/Kim Hyun Joong (SS501-OST Boys over flowers) - Nae meoriga nabbaseo ( Because I'm Stupid).mp3",
		"K-pop old/miss A (    ) - (Come On Over).mp3",
		"K-pop old/NS Yoon-G feat. Jay Park - If You Love Me.mp3",
		"K-pop old/Ost my girlfriend Kumiho - No Min Woo-Trap.mp3",
		"K-pop old/SNSD (Girls' Generation) - Mr.Taxi (Korean Ver.).mp3",
		"K-pop old/SNSD (Girls' Generation) - The Boys.mp3",
		"K-pop old/T-ARA - Holiday.mp3",
		"K-pop old/Yanghwajin (OST City hunter Городской охотник, 2011) - It's Alright.mp3"
    };*/
	
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

