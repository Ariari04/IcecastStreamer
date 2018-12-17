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

		Mp3Encoder();
		~Mp3Encoder();

		bool open(const char* fileName) override;
		bool open(const wchar_t* fileName) override;

		int write(const void* SrcBuf, size_t ElementSize, size_t Count, FILE* outFile) override;
	};

}
