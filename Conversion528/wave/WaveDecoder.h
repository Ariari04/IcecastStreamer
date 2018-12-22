#pragma once

#include <cstdio>
#include <AudioFile.h>

#include <wave/WaveFile.h>

namespace Decoding
{

	class WaveDecoder : public AudioDecoder
	{
	public:
		FILE* file = NULL;
		WaveFileChunks::WaveFileHeader Header;

		~WaveDecoder();

		void close();

		int open(const char* fileName) override;
		int read(char* Buffer, size_t Count) override;
	};

}