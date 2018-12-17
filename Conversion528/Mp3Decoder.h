#pragma once

#include <lame.h>

#include <cstdio>
#include <vector>
#include <AudioFile.h>
#include "Progress.h"

#include <Conversion.h>

typedef struct mpstr_tag MPSTR;

namespace Decoding
{

	class Mp3Decoder : public AudioFileReader // public BufferedProducerBase
	{
	public:
		lame_t gf;
		bool endOfFile;

		Mp3Decoder();
		~Mp3Decoder();

		bool open(const char* fileName) override;
		bool open(const wchar_t* fileName) override;

		int read(void* DstBuf, size_t ElementSize, size_t Count, FILE* outFile) override;
		int isEof() override;
	};

}