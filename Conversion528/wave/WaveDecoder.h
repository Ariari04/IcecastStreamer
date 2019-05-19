#pragma once

#include <cstdio>
#include <wave/WaveFile.h>
#include <AudioFile.h>
#include <fstream>

#include <lame_imported/imported.h>


namespace Decoding
{

	class WaveDecoder : public AudioDecoder
	{
	public:
		FILE* file = NULL;
		WaveFileChunks::WaveFileHeader Header;

		~WaveDecoder();

		void close();

		int open(const char* fileName) override;
		int read(char* Buffer, size_t Count) override;
	};



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
		int read(char* Buffer, size_t Count) override;

		int openMp3Output();

		int readDurationOld(char* Buffer, size_t Count, std::chrono::seconds duration);

		int readDuration(char* Buffer, size_t Count, std::chrono::seconds duration) override;
	};


}