#pragma once

#include <stdio.h>
#include <chrono>

class AudioDecoder
{
public:
	virtual ~AudioDecoder() = default;

	virtual int open(const char* fileName) = 0;
	virtual int readDuration(char* Buffer, size_t Count, std::chrono::seconds duration) { return 0; }
};

