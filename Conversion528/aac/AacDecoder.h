#pragma once

#include <cstdio>
#include <AudioFile.h>

#include <lame.h>
#include <vector>
#include <array>
#include <fstream>

#include <neaacdec.h>


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


	class AacToMp3Decoder : public AudioDecoder // public BufferedProducerBase
	{
	public:
		
		size_t input_size = 0;

		std::ifstream f;

		std::array<unsigned char, 1024 * 1024> input;

		int totalRead = 0;

		std::array<short, 1024 * 1024*8> pcmBuffer;
		//std::array<short, 266240> pcmBuffer;

		

		NeAACDecHandle hAac = NeAACDecOpen();

		unsigned long samplerate = 0;
		unsigned char channels = 0;
		size_t curIndex = 0;

		lame_t lame = nullptr;

		bool flush_sent = false;

		AacToMp3Decoder();
		~AacToMp3Decoder();

		int open(const char* fileName) override;

		int read(char* Buffer, size_t Count) override;

		int readDuration(char* Buffer, size_t Count, std::chrono::seconds duration) override;

		int openMp3Output();
	};

}