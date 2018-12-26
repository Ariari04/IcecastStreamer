#pragma once

#include <cstdio>
#include <wave/WaveFile.h>
#include <AudioFile.h>

namespace Encoding
{

	class WaveEncoder : public AudioEncoder
	{
	public:
		FILE * file = NULL;
		WaveFileChunks::WaveFileHeader Header;

		~WaveEncoder();

		void close();

		int open(const char* fileName);
		int write(char* Buffer, size_t Count);
	};
}
