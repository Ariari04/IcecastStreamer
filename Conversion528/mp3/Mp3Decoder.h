#pragma once

#include <lame.h>

#include <cstdio>
#include <AudioFile.h>
#include <array>
#include <vector>
#include <fstream>

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


	class Mp3WaveMp3Decoder : public AudioDecoder // public BufferedProducerBase
	{
	public:
		hip_t lameInput;

		std::ifstream f;

		std::array<char, 1024 * 1024> buffer;

		std::array<short, 1024 * 1024 * 8> pcm_l;
		std::array<short, 1024 * 1024 * 8> pcm_r;
		
		int pcmSize = 0;

		mp3data_struct mp3data{ 0 };

		lame_t lame = nullptr;

		bool flush_sent = false;

		Mp3WaveMp3Decoder();
		~Mp3WaveMp3Decoder();

		int open(const char* fileName) override;
		int read(char* Buffer, size_t Count) override;

		void close();

		int innerRead();

		int readDuration(char* Buffer, size_t Count, std::chrono::seconds duration) override;
		int openMp3Output();
	};
}