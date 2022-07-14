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
	//extern std::array<char, BLOCK_SIZE> buffer;

	//extern std::array<short, BLOCK_SIZE * 64> pcm_l;
	//extern std::array<short, BLOCK_SIZE * 64> pcm_r;

	extern std::array<short, BLOCK_SIZE * 64> pcm_all;

	extern std::array<short, BLOCK_SIZE * 64> tempBuf;


	class AacToMp3Decoder : public AudioDecoder 
	{
	public:

		std::ifstream f;

		size_t bufferStartPos = 0;
		//std::array<unsigned char, 1024 * 128> buffer;

		int pcmSize = 0;

		//std::array<short, 1024 * 1024*8> pcmBuffer;

		bool aacInited = false;

		NeAACDecHandle hAac = NeAACDecOpen();

		unsigned long samplerate = 0;
		unsigned char channels = 0;

		lame_t lame = nullptr;

		bool flush_sent = false;

		AacToMp3Decoder();
		~AacToMp3Decoder();

		int open(const char* fileName) override;

		int readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration) override;

		int openMp3Output();

		int innerRead();

		void close();
	};

	//------------------------

	class AacDecoder : public AudioDecoderInterface
	{
	public:
		std::ifstream f;

		int curIndex = 0;

		size_t bufferStartPos = 0;
		//std::array<unsigned char, 1024 * 128> buffer;

		int pcmSize = 0;
		int pcmShift = 0;

		//std::array<short, 1024 * 1024*8> pcmBuffer;

		bool aacInited = false;

		NeAACDecHandle hAac = NeAACDecOpen();

		unsigned long samplerate = 0;
		unsigned char channels = 0;

		~AacDecoder();

		virtual int readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration, std::chrono::milliseconds& actualDurationRead) override;

		void close();

		virtual bool open(const char* fileName) override;
	};

}
