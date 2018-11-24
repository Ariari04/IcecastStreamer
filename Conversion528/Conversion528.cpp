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

void ProcessFileName(const std::string& fileName)
{
	WaveFile::WaveFileReader reader;


	if (!reader.open(fileName.c_str()))
	{
#ifdef _WRITE_ERROR_DESCR_
		std::ofstream ofs("result.txt");
		KString str;
		str.Format("Input was not opened (%d): ", reader.getLastError());
		str += WFREToString(reader.getLastError());
		ofs << (const char*)str;
		ofs.close();
#endif

		return;
	}


	WaveFile::WaveProducer producer(reader);

	std::string outputFileName(fileName.begin(), fileName.begin() + fileName.size() - 4);
	outputFileName += ".png";
	

	Conversion::ConversionResult res = Conversion::ConversionOnlySpectral(producer, outputFileName);

#ifdef _WRITE_ERROR_DESCR_
	std::ofstream ofs("result.txt");
	ofs << (const char*)ConversionResultToString(res);
	ofs.close();
#endif

	reader.close();

}

int main(int argc, char* argv[])
{
	
	//const char* fileName = "D:\\Work\\TorchProjects\\converter\\Conversion528\\Track5.wav";

	//const char* fileName = "D:\\Work\\TorchProjects\\converter\\Conversion528\\usp_unsil-1.wav";
	//const char* fileName = "D:\\Work\\TorchProjects\\converter\\Conversion528\\vip.wav";
	//const char* fileName = "D:\\Work\\TorchProjects\\converter\\Conversion528\\tone.wav";
	const char* fileName = "D:\\IHearVoicesData\\Audio\\device1_channel1_20181012124210.wav";

	const char* folderName = "D:\\IHearVoicesData\\Audio\\test";


	auto fileNames = getFileNamesInFolder(folderName);

	for (size_t i = 0; i < fileNames.size(); i++)
	{
		std::string fileName = fileNames[i];
		std::string substr = fileName.substr(fileName.size() - 4, 4);
		if (substr == ".wav" /*&& fileName == "D:\\IHearVoicesData\\Audio\\bad_audio\\device1_channel1_20181023090834.wav"*/)
		{
			ProcessFileName(fileName);
		}
	}

	return 0;
}

