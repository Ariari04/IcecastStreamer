#pragma once

#include <cstdio>
#include <wave/WaveFile.h>
#include <AudioFile.h>

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