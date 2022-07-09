#pragma once

/*
#include <lame.h>
*/
#include <cstdio>
#include <AudioFile.h>

#include <array>
#include <vector>
#include <fstream>


#include "minimp3.h"

struct MP3DHolder
{
	mp3dec_t mp3d;

	MP3DHolder()
	{
		mp3dec_init(&mp3d);
	}
};

namespace DecodingX
{
	
	extern std::array<char, BLOCK_SIZE> buffer;

	extern std::array<short, BLOCK_SIZE * 64> pcm_l;
	extern std::array<short, BLOCK_SIZE * 64> pcm_r;

	extern std::array<short, BLOCK_SIZE * 64> tempBuf;


	class Mp3WaveMp3DecoderNew : public AudioDecoder // public BufferedProducerBase
	{
	public:
		
		//hip_t lameInput;

		std::ifstream f;


		static MP3DHolder mp3dHolder;
		

		
		
		//int pcmSize = 0;

		//mp3data_struct mp3data{ 0 };

		//lame_t lame = nullptr;

		//bool flush_sent = false;

		Mp3WaveMp3DecoderNew();
		~Mp3WaveMp3DecoderNew();

		int open(const char* fileName) override;
	
		void close();

		int innerRead();

		int readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration) override;
		int openMp3Output();
	};
}