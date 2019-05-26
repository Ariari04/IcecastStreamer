#pragma once

#include <cstdio>
#include <AudioFile.h>

#include <lame.h>
#include <vector>
#include <array>
#include <fstream>

#include <neaacdec.h>
#include <cstring>

namespace Decoding
{


	class AacToMp3Decoder : public AudioDecoder 
	{
	public:

		std::ifstream f;

		size_t bufferStartPos = 0;
		std::array<unsigned char, 1024 * 128> buffer;

		int pcmSize = 0;

		std::array<short, 1024 * 1024*8> pcmBuffer;

		bool aacInited = false;

		NeAACDecHandle hAac = NeAACDecOpen();

		unsigned long samplerate = 0;
		unsigned char channels = 0;

		lame_t lame = nullptr;

		bool flush_sent = false;

		AacToMp3Decoder();
		~AacToMp3Decoder();

		int open(const char* fileName) override;

		int readDuration(char* Buffer, size_t Count, std::chrono::seconds duration) override;

		int openMp3Output();

		int innerRead();

		void close();
	};

}
