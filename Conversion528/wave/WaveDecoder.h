#pragma once

#include <cstdio>
#include <wave/WaveFile.h>
#include <AudioFile.h>
#include <fstream>
#include "lame.h"

namespace Decoding
{
	   
	class WaveToMp3Decoder : public AudioDecoder
	{
	public:
		
		std::ifstream f;

		WaveFileChunks::WaveFileHeader Header;

		lame_t lame = nullptr;

		bool flush_sent = false;

		~WaveToMp3Decoder();

		void close();

		int open(const char* fileName) override;

		int openMp3Output();

		int readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration) override;
	};


}