#include "Mp3DecoderNew.h"

#include <fstream>

#include <iostream>
#include <cstring>


#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"

size_t getID3TagSize(const char* filename);


namespace DecodingX
{

	//std::array<char, BLOCK_SIZE> buffer;
	std::array<char, 1024*1024> buffer;

	std::array<short, BLOCK_SIZE * 64> pcm_l;
	std::array<short, BLOCK_SIZE * 64> pcm_r;

	std::array<short, BLOCK_SIZE * 64> tempBuf;

	MP3DHolder Mp3WaveMp3DecoderNew::mp3dHolder;
	
	Mp3WaveMp3DecoderNew::Mp3WaveMp3DecoderNew()
	{
		//lameInput = hip_decode_init();
	}

	Mp3WaveMp3DecoderNew::~Mp3WaveMp3DecoderNew()
	{
		/*if (lame != nullptr)
		{
			lame_close(lame);
			lame = nullptr;
		}
		hip_decode_exit(lameInput);*/
	}

	void Mp3WaveMp3DecoderNew::close()
	{
		f.close();
	}

	/*
	void reportf(const char *format, va_list ap)
	{
		std::cout << format << std::endl;
	}*/

	

	bool Mp3WaveMp3DecoderNew::open(const char* fileName)
	{

		
		f.close();
		/*
		size_t mp3TagSize = getID3TagSize(fileName);

		if (lame != nullptr)
		{
			lame_close(lame);
			lame = nullptr;
		}

		f.open(fileName, std::ios::binary);

		f.seekg(mp3TagSize);
		*/

		f.open(fileName, std::ios::binary);

		f.read(reinterpret_cast<char*>(&buffer[0]), buffer.size());

		auto count = f.gcount();

		if (count != buffer.size())
		{
			close();
			return false;
		}
		
		return true;
	}


	int Mp3WaveMp3DecoderNew::readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration, std::chrono::milliseconds& actualDurationRead)
	{

		mp3dec_frame_info_t info;
		static short pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
		/*unsigned char *input_buf; - input byte stream*/


	

		//fileIsOver = f.eof();


		bool fileIsOver = false;


		bool audioDataFound = false;

		int samples = 0;

		while (!audioDataFound)
		{
		
			samples = mp3dec_decode_frame(&mp3dHolder.mp3d, reinterpret_cast<uint8_t*>(&buffer[bufferPos]), buffer.size() - bufferPos, pcm, &info);

			if (samples > 0)
			{
				audioDataFound = true;
			}
			else if (samples == 0)
			{
				//Insufficient data, decode more
			}

			bufferPos += info.frame_bytes;

		}


		int write = samples * info.channels * 2;

		memcpy(Buffer, &pcm[0], write);

		actualDurationRead = duration;

		return write;





		/*
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
		*/

		//return write;
	}
}
