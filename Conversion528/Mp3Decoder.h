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
		FILE* oufFile;
		bool endOfFile;

		Mp3Decoder();
		~Mp3Decoder();

		int open(const char* fileName) override;
		int open(const wchar_t* fileName) override;

		int read(char Buffer[2 * 1152 * 2], FILE* outFile) override;
		int isEof() override;
	};

}