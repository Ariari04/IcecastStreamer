#pragma once

#include <cstdio>
#include <AudioFile.h>

namespace Decoding
{

	class AacDecoder : public AudioDecoder // public BufferedProducerBase
	{
	public:
		bool firstZero;
		int mp4SampleCount;

		AacDecoder();
		~AacDecoder();

		int open(const char* fileName) override;

		int read(char* Buffer, size_t Count) override;
	};

}