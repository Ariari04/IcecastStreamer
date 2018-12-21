#pragma once

#include <stdio.h>

class AudioFileReader
{
public:
	virtual ~AudioFileReader() = default;

	virtual int open(const wchar_t* strFile) = 0;
	virtual int open(const char* strFile) = 0;

	virtual int read(char Buffer[2 * 1152 * 2], FILE* outFile) = 0;
	virtual int isEof() = 0;
};

class AudioFileWriter
{
public:
	virtual ~AudioFileWriter() = default;

	virtual int open(const wchar_t* strFile) = 0;
	virtual int open(const char* strFile) = 0;

	virtual int write(int Buffer[2][1152], size_t ElementSize, size_t Count, FILE* outFile) = 0;
};