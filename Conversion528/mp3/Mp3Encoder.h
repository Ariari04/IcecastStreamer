#pragma once

#include <AudioFile.h>

#include <lame.h>

namespace Encoding
{

	class Mp3Encoder : public AudioEncoder // public EncoderConsumerBase
	{
	public:
		lame_t gf;
		FILE* outFile;
		size_t  id3v2_size;

		Mp3Encoder();
		~Mp3Encoder();

		int open(const char* inFileName, const char* outFileName) override;
		int write(char* Buffer, size_t Count) override;
	};

}
