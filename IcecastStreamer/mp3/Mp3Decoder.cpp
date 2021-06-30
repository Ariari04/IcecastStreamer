#include "Mp3Decoder.h"

#include <fstream>

#include <iostream>
#include <cstring>

size_t getID3TagSize(const char* filename);


namespace Decoding
{

	std::array<char, BLOCK_SIZE> buffer;

	std::array<short, BLOCK_SIZE * 64> pcm_l;
	std::array<short, BLOCK_SIZE * 64> pcm_r;

	std::array<short, BLOCK_SIZE * 64> tempBuf;
	
	Mp3WaveMp3Decoder::Mp3WaveMp3Decoder()
	{
		lameInput = hip_decode_init();
	}

	Mp3WaveMp3Decoder::~Mp3WaveMp3Decoder()
	{
		if (lame != nullptr)
		{
			lame_close(lame);
			lame = nullptr;
		}
		hip_decode_exit(lameInput);
	}

	void Mp3WaveMp3Decoder::close()
	{
		f.close();
	}

	void reportf(const char *format, va_list ap)
	{
		std::cout << format << std::endl;
	}

	int Mp3WaveMp3Decoder::innerRead()
	{
		if (!f)
		{
			return 0;
		}

		bool fileIsOver = false;

		f.read(reinterpret_cast<char*>(&buffer[0]), buffer.size());

		auto count = f.gcount();

		if (count != buffer.size())
		{
			close();
		}

		fileIsOver = f.eof();

		hip_set_errorf(lameInput, reportf);

		int decodeResult = hip_decode_headers(lameInput, reinterpret_cast<unsigned char*>(&buffer[0]), count, &pcm_l[pcmSize], &pcm_r[pcmSize], &mp3data);

		while ((decodeResult == 0) && (fileIsOver == false))
		{
			f.read(reinterpret_cast<char*>(&buffer[0]), buffer.size());

			auto count = f.gcount();

			decodeResult = hip_decode_headers(lameInput, reinterpret_cast<unsigned char*>(&buffer[0]), count, &pcm_l[pcmSize], &pcm_r[pcmSize], &mp3data);

		}

		if (decodeResult == -1)
		{
			std::cout << "decodeResult error: " << decodeResult << std::endl;
		}
		else
		{
			pcmSize += decodeResult;

			std::cout << "decodeResult: " << decodeResult << std::endl;

		}

		return 0;
	}

	int Mp3WaveMp3Decoder::open(const char* fileName)
	{

		f.close();

		size_t mp3TagSize = getID3TagSize(fileName);

		if (lame != nullptr)
		{
			lame_close(lame);
			lame = nullptr;
		}

		f.open(fileName, std::ios::binary);

		f.seekg(mp3TagSize);

		innerRead();

		openMp3Output();

		return 1;
	}

	int Mp3WaveMp3Decoder::openMp3Output()
	{
		lame = lame_init();
		lame_set_in_samplerate(lame, this->mp3data.samplerate);
		lame_set_VBR(lame, vbr_default);
		lame_init_params(lame);

		return 1;
	}

	int Mp3WaveMp3Decoder::readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration)
	{
		auto readCount = duration.count() * this->mp3data.samplerate / 1000;

		if (pcmSize < readCount)
		{
			std::cout << "buffers going low, refill..." << std::endl;
			innerRead();

			if (pcmSize < readCount)
			{
				std::cout << "after refill buffers are still low. Closing soon" << std::endl;
				readCount = pcmSize;
			}
		}


		if (pcmSize == 0)
		{
			if (!flush_sent)
			{
				flush_sent = true;
				return lame_encode_flush(lame, reinterpret_cast<unsigned char*>(Buffer), Count);
			}
			else
			{
				return 0;
			}
		}

		int write = lame_encode_buffer(lame, &pcm_l[0], &pcm_r[0], readCount, reinterpret_cast<unsigned char*>(Buffer), Count);

		auto leftoverCount = pcmSize - readCount;

		if (leftoverCount > 0)
		{

			//std::vector<short> tempBuf;
			//tempBuf.resize(leftoverCount);

			std::memcpy(&tempBuf[0], &pcm_l[readCount], leftoverCount * sizeof(short));

			std::memcpy(&pcm_l[0], &tempBuf[0], leftoverCount * sizeof(short));


			std::memcpy(&tempBuf[0], &pcm_r[readCount], leftoverCount * sizeof(short));

			std::memcpy(&pcm_r[0], &tempBuf[0], leftoverCount * sizeof(short));

		}

		pcmSize = leftoverCount;

		return write;
	}
}
