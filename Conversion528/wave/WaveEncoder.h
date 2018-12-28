#pragma once

#include <cstdio>
#include <wave/WaveFile.h>
#include <AudioFile.h>

namespace Encoding
{

	class WaveEncoder : public AudioEncoder
	{
	public:
		FILE * inFile = NULL;
		FILE * outFile = NULL;
		WaveFileChunks::WaveFileHeader Header;

		~WaveEncoder();

		void close();

		int open(const char* inFileName, const char* outFileName);
		int write(char* Buffer, size_t Count);
	};
}
