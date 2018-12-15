#define _CRT_SECURE_NO_DEPRECATE

#include <fstream>
#include <MP3File.h>

//#include <stdio.h>
//#include <string.h>
//#include <cassert>

bool MP3File::MP3FileReader::open(const wchar_t* strFileName)
{
	decoder.open(strFileName);
	return true;
}

bool MP3File::MP3FileReader::open(const char* strFileName)
{
	decoder.open(strFileName);
	return true;
}

size_t MP3File::MP3FileReader::MusicFread(void* DstBuf, size_t ElementSize, size_t Count)
{
	std::vector<byte> buffer(255);
	decoder.readSamples(buffer);
	
	std::ofstream ofs("out.txt");
	ofs << buffer.data();

	return 0;
}

int MP3File::MP3FileReader::MusicFeof()
{
	return 0;
}



bool MP3File::MP3FileWriter::open(const wchar_t* strFileName)
{
	return false;
}

bool MP3File::MP3FileWriter::open(const char* strFileName)
{
	return false;
}

bool MP3File::MP3FileWriter::writeHeader()
{
	return false;
}

size_t MP3File::MP3FileWriter::MusicFwrite(const void* SrcBuf, size_t ElementSize, size_t Count)
{
	return 0;
}