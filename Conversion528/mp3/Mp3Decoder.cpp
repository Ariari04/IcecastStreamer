#include "Mp3Decoder.h"

#include <fstream>

#include <lame_imported/imported.h>

#include <iostream>

namespace Decoding
{
	Mp3Decoder::Mp3Decoder()
	{
		oufFile = NULL;
		gf = lame_init(); /* initialize libmp3lame */
	}

	Mp3Decoder::~Mp3Decoder()
	{
		if (oufFile)
		{
			fclose(oufFile);       /* close the output file */
			oufFile = NULL;
			lame_decoding_close();
		}

		lame_close(gf);
	}

	int Mp3Decoder::open(const char* fileName)
	{
		wavsize = 0;

		std::string additionalOutputFile = std::string(fileName) + ".wav";
		int argc = 5;
		char* argv[5] = { "", "--decode", "-t", (char*)fileName, (char*)additionalOutputFile.c_str() };
		if (lame_main_imported(gf, argc, argv, &oufFile, 0))
		{
			return 0;
		}

		if (open_decoding(gf, &oufFile))
		{
			return 0;
		}

		return 1;
	}

	int Mp3Decoder::read(char* Buffer, size_t Count)
	{
		
		int iread = lame_decoder_iter(gf, oufFile, Buffer, Count, &wavsize);

		if (iread < 1)
		{
			lame_decoding_close();
			if (oufFile)
			{
				fclose(oufFile); 
				oufFile = NULL;
			}
			return -1;
		}

		return iread;
	}


	//----------------------------------


	Mp3WaveMp3Decoder::Mp3WaveMp3Decoder()
	{
		lameInput = hip_decode_init();
	}

	Mp3WaveMp3Decoder::~Mp3WaveMp3Decoder()
	{
		hip_decode_exit(lameInput);
	}

	void Mp3WaveMp3Decoder::close()
	{
		f.close();
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

		int decodeResult = hip_decode_headers(lameInput, reinterpret_cast<unsigned char*>(&buffer[0]), count, &pcm_l[pcmSize], &pcm_r[pcmSize], &mp3data);

		while ((decodeResult == 0) && (fileIsOver == false))
		{
			f.read(reinterpret_cast<char*>(&buffer[0]), buffer.size());

			auto count = f.gcount();

			decodeResult = hip_decode_headers(lameInput, reinterpret_cast<unsigned char*>(&buffer[0]), count, &pcm_l[pcmSize], &pcm_r[pcmSize], &mp3data);

		}

		pcmSize += decodeResult;

		std::cout << "decodeResult: " << decodeResult << std::endl;

		return 0;
	}

	int Mp3WaveMp3Decoder::open(const char* fileName)
	{

		f.close();

		f.open(fileName, std::ios::binary);

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

	int Mp3WaveMp3Decoder::read(char* Buffer, size_t Count)
	{
		return 0;
		/*
		int iread = lame_decoder_iter(gf, oufFile, Buffer, Count, &wavsize);

		if (iread < 1)
		{
			lame_decoding_close();
			if (oufFile)
			{
				fclose(oufFile);
				oufFile = NULL;
			}
			return -1;
		}

		return iread;*/
	}

	int Mp3WaveMp3Decoder::readDuration(char* Buffer, size_t Count, std::chrono::seconds duration)
	{
		auto readCount = duration.count() * this->mp3data.samplerate;// * this->mp3data.stereo;

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

			std::vector<short> tempBuf;
			tempBuf.resize(leftoverCount);

			std::memcpy(&tempBuf[0], &pcm_l[readCount], leftoverCount * sizeof(short));

			std::memcpy(&pcm_l[0], &tempBuf[0], leftoverCount * sizeof(short));


			std::memcpy(&tempBuf[0], &pcm_r[readCount], leftoverCount * sizeof(short));

			std::memcpy(&pcm_r[0], &tempBuf[0], leftoverCount * sizeof(short));

		}

		pcmSize = leftoverCount;

		return write;

		/*

		for (size_t i = 0; i < readCount; i++)
		{
			std::memcpy(&Buffer[2 * (2 * i)], &pcm_l[i], 2);
			std::memcpy(&Buffer[2 * (2 * i + 1)], &pcm_r[i], 2);
		}

		size_t leftoverCount = pcmSize - readCount;

		std::memmove(&pcm_l[0], &pcm_l[readCount], pcmSize);
		std::memmove(&pcm_r[0], &pcm_r[readCount], pcmSize);

		pcmSize = leftoverCount;

		return readCount;*/
	}
}
