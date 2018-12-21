#pragma once

#include <AudioFile.h>

#include <lame.h>

namespace Encoding
{

	class Mp3Encoder : public AudioFileWriter // public EncoderConsumerBase
	{
	public:
		lame_t gf;
		bool writeHeader;
		FILE* outFile;
		size_t  id3v2_size;

		Mp3Encoder();
		~Mp3Encoder();

		int open(const char* fileName) override;
		int open(const wchar_t* fileName) override;

		int write(int Buffer[2][1152], size_t ElementSize, size_t Count, FILE* outFile) override;
	};

}
