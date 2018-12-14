/* Copyright Zaurmann Software 2010 */
#pragma once

//#include "StaticAssertion.h"
//#include "IntegerTypes.h"
//#include "KString.h"

#include <AudioFile.h>

namespace MP3File
{

class MP3FileReader : public AudioFileReader
{
public:

	bool open(const wchar_t* strFileName) override;
	bool open(const char* strFileName) override;

	bool close();
	bool isOpened() const;

	size_t MusicFread(void* DstBuf, size_t ElementSize, size_t Count) override;
	int MusicFeof() override;
};

class MP3FileWriter : public AudioFileWriter
{
public:
	bool open(const wchar_t* strFileName);
	bool open(const char* strFileName);

	bool close();
	bool isOpened() const;

	bool writeHeader() override;
	size_t MusicFwrite(const void* SrcBuf, size_t ElementSize, size_t Count) override;
};

}