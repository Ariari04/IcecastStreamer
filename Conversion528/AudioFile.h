#pragma once

#include <stdio.h>

class AudioFileReader
{
public:
	virtual ~AudioFileReader() = default;

	virtual bool open(const wchar_t* strFile) = 0;
	virtual bool open(const char* strFile) = 0;

	virtual int read(void* DstBuf, size_t ElementSize, size_t Count, FILE* outFile) = 0;
	virtual int isEof() = 0;
};

class AudioFileWriter
{
public:
	virtual ~AudioFileWriter() = default;

	virtual bool open(const wchar_t* strFile) = 0;
	virtual bool open(const char* strFile) = 0;

	virtual int write(const void* SrcBuf, size_t ElementSize, size_t Count, FILE* outFile) = 0;
};