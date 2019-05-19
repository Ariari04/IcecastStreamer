#pragma once

#include <stdio.h>
#include <chrono>

class AudioDecoder
{
public:
	virtual ~AudioDecoder() = default;

	virtual int open(const char* fileName) = 0;
	virtual int read(char* Buffer, size_t Count) = 0;
	virtual int readDuration(char* Buffer, size_t Count, std::chrono::seconds duration) { return 0; }
};

class AudioEncoder
{
public:
	virtual ~AudioEncoder() = default;

	virtual int open(const char* inFileName, const char* outFileName) = 0;
	virtual int write(char* Buffer, size_t Count) = 0;
};