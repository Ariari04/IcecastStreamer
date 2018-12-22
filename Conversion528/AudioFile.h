#pragma once

#include <stdio.h>

class AudioDecoder
{
public:
	virtual ~AudioDecoder() = default;

	virtual int open(const char* fileName) = 0;
	virtual int read(char* Buffer, size_t Count) = 0;
};

class AudioEncoder
{
public:
	virtual ~AudioEncoder() = default;

	virtual int open(const char* fileName) = 0;
	virtual int write(char* Buffer, size_t Count) = 0;
};