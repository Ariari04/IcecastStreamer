#pragma once

#include <cstdio>
#include <AudioFile.h>

#include <vector>
#include <array>
#include <fstream>

#include <neaacdec.h>
#include <cstring>

namespace Decoding
{

	class AacDecoder : public AudioDecoderInterface
	{
	public:
		std::ifstream f;

		int curIndex = 0;

		size_t bufferStartPos = 0;
		std::array<char, 1024 * 128> inputFileBuffer;
		size_t inputFileBufferSize = inputFileBuffer.size();

		int pcmSize = 0;
		int pcmShift = 0;

		//std::array<short, 1024 * 1024*8> pcmBuffer;

		std::array<short, BLOCK_SIZE * 64> outputPcmBuffer;

		std::array<short, 1024 * 1024> pcm_all;

		bool aacInited = false;

		bool fileIsOver = false;

		NeAACDecHandle hAac = NeAACDecOpen();

		unsigned long samplerate = 0;
		unsigned char channels = 0;

		~AacDecoder();

		virtual int readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration, std::chrono::milliseconds& actualDurationRead) override;

		void close();

		virtual bool open(const char* fileName) override;
	};

}
