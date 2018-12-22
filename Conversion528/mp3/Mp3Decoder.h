#pragma once

#include <lame.h>

#include <cstdio>
#include <AudioFile.h>

//#include <Conversion.h>

namespace Decoding
{

	class Mp3Decoder : public AudioDecoder // public BufferedProducerBase
	{
	public:
		lame_t gf;
		double wavsize;
		FILE* oufFile;

		Mp3Decoder();
		~Mp3Decoder();

		int open(const char* fileName) override;
		//int open(const wchar_t* fileName) override;

		int read(char* Buffer, size_t Count) override;
	};

}