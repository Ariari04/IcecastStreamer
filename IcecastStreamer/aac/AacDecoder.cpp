#include <aac/AacDecoder.h>

#include <string.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace Decoding
{
	//---------------------------------------

	//std::array<short, BLOCK_SIZE * 64> pcm_all;
	std::array<short, 1024 * 1024> pcm_all;

	std::array<char, 1024*1024> aacBuffer;

	AacToMp3Decoder::AacToMp3Decoder()
	{
		
	}

	AacToMp3Decoder::~AacToMp3Decoder()
	{
		if (aacInited)
		{
			NeAACDecClose(hAac);
		}
	}

	int AacToMp3Decoder::open(const char* fileName)
	{

		// Get the current config
		NeAACDecConfigurationPtr conf = NeAACDecGetCurrentConfiguration(hAac);

		// XXX: If needed change some of the values in conf

		conf->outputFormat = FAAD_FMT_16BIT;

		// Set the new configuration
		NeAACDecSetConfiguration(hAac, conf);

		f.close();

		f.open(fileName, std::ios::binary);

		innerRead();

		openMp3Output();

		return 1;
	}


	int AacToMp3Decoder::innerRead()
	{
		if (!f)
		{
			return 0;
		}

		bool fileIsOver = false;

		auto mustReadCount = aacBuffer.size() - bufferStartPos;

		f.read(reinterpret_cast<char*>(&aacBuffer[bufferStartPos]), mustReadCount);

		auto readCount = f.gcount();

		if (readCount != mustReadCount)
		{
			close();
		}

		fileIsOver = f.eof();

		auto count = readCount + bufferStartPos;

		
		if (!aacInited)
		{
		
			char err = NeAACDecInit(hAac, reinterpret_cast<unsigned char*>(&aacBuffer[0]), count, &samplerate, &channels);
			if (err != 0) {
				// Handle error
				//fprintf(stderr, "NeAACDecInit error: %d\n", err);
				return 0;
			}

			aacInited = true;
		}

		NeAACDecFrameInfo hInfo;
		memset(&hInfo, 0, sizeof hInfo);

		bool processedAll = false;

		size_t curIndex = 0;

		while (!processedAll)
		{

			auto output = NeAACDecDecode(hAac, &hInfo, reinterpret_cast<unsigned char*>(&aacBuffer[curIndex]), count);

			if ((hInfo.error == 0) && (hInfo.samples > 0)) {
				// do what you need to do with the decoded samples
				/*
				fprintf(stderr, "decoded %lu samples\n", hInfo.samples);
				fprintf(stderr, "  bytesconsumed: %lu\n", hInfo.bytesconsumed);
				fprintf(stderr, "  channels: %d\n", hInfo.channels);
				fprintf(stderr, "  samplerate: %lu\n", hInfo.samplerate);
				fprintf(stderr, "  sbr: %u\n", hInfo.sbr);
				fprintf(stderr, "  object_type: %u\n", hInfo.object_type);
				fprintf(stderr, "  header_type: %u\n", hInfo.header_type);
				fprintf(stderr, "  num_front_channels: %u\n", hInfo.num_front_channels);
				fprintf(stderr, "  num_side_channels: %u\n", hInfo.num_side_channels);
				fprintf(stderr, "  num_back_channels: %u\n", hInfo.num_back_channels);
				fprintf(stderr, "  num_lfe_channels: %u\n", hInfo.num_lfe_channels);
				fprintf(stderr, "  ps: %u\n", hInfo.ps);
				fprintf(stderr, "\n");
				*/

				curIndex += hInfo.bytesconsumed;
				count -= hInfo.bytesconsumed;
				//fprintf(stderr, "%zd %zd\n", curIndex, count);

				auto readBytes = hInfo.samples* hInfo.channels;

				if (readBytes != 0)
				{

					std::memcpy(reinterpret_cast<char*>(&pcm_all[0]) + pcmSize, output, readBytes);

				}

				pcmSize += readBytes;

			}
			else if (hInfo.error != 0) {
				// Some error occurred while decoding this frame
				fprintf(stderr, "NeAACDecode error: %d\n", hInfo.error);
				fprintf(stderr, "%s\n", NeAACDecGetErrorMessage(hInfo.error));

				/*
				if ((hInfo.error == 15) || (hInfo.error == 13))
				{
					processedAll = true;

					std::vector<short> tempBuf;
					tempBuf.resize(count);

					std::memcpy(reinterpret_cast<char*>(&tempBuf[0]), reinterpret_cast<char*>(&buffer[0]) + curIndex, count);

					std::memcpy(reinterpret_cast<char*>(&buffer[0]), reinterpret_cast<char*>(&tempBuf[0]), count);

					bufferStartPos = count;
				}
				else
				{
					pcmSize = 0;
					return 0;
				}*/

				pcmSize = 0;
				return 0;

				
				
			}
			else {
				//fprintf(stderr, "got nothing...\n");
			}

			
			if ((count < 1024) && (!fileIsOver))
			{
				processedAll = true;

				//std::vector<short> tempBuf;
				//tempBuf.resize(count);

				std::memcpy(reinterpret_cast<char*>(&tempBuf[0]), reinterpret_cast<char*>(&aacBuffer[0]) + curIndex, count);

				std::memcpy(reinterpret_cast<char*>(&aacBuffer[0]), reinterpret_cast<char*>(&tempBuf[0]), count);

				bufferStartPos = count;
			}

			if ((count == 0) && fileIsOver)
			{
				processedAll = true;
			}
		}

		return 0;
	}

	void AacToMp3Decoder::close()
	{
		f.close();
	}


	int AacToMp3Decoder::readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration)
	{
		
		auto readCount = duration.count() * samplerate*channels*2 / 1000;


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

	    int write = lame_encode_buffer_interleaved(lame, &pcm_all[0], readCount/(channels*2), reinterpret_cast<unsigned char*>(Buffer), Count);
		
		auto leftoverCount = pcmSize - readCount;

		if (leftoverCount > 0)
		{

			//std::vector<short> tempBuf;
			//tempBuf.resize(leftoverCount);

			std::memcpy(reinterpret_cast<char*>(&tempBuf[0]), reinterpret_cast<char*>(&pcm_all[0]) + readCount, leftoverCount * sizeof(short));

			std::memcpy(reinterpret_cast<char*>(&pcm_all[0]), reinterpret_cast<char*>(&tempBuf[0]), leftoverCount * sizeof(short));

		}

		pcmSize = leftoverCount;
		
		return write;

	}


	int AacToMp3Decoder::openMp3Output()
	{
		lame = lame_init();
		lame_set_in_samplerate(lame, samplerate);
		lame_set_VBR(lame, vbr_default);
		lame_init_params(lame);

		return 1;
	}

	//-----------------------------
	//------------------------

	AacDecoder::~AacDecoder()
	{
		close();
	}

	int AacDecoder::readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration, std::chrono::milliseconds& actualDurationRead)
	{
	
		pcmSize = 0;

		int secondSize = 176400;

		while (pcmSize < secondSize)
		{
			auto count = 1024 * 1024 - curIndex;

			NeAACDecFrameInfo hInfo;
			memset(&hInfo, 0, sizeof hInfo);

			auto output = NeAACDecDecode(hAac, &hInfo, reinterpret_cast<unsigned char*>(&aacBuffer[curIndex]), count);

			if ((hInfo.error == 0) && (hInfo.samples > 0)) {
				// do what you need to do with the decoded samples
				/*
				fprintf(stderr, "decoded %lu samples\n", hInfo.samples);
				fprintf(stderr, "  bytesconsumed: %lu\n", hInfo.bytesconsumed);
				fprintf(stderr, "  channels: %d\n", hInfo.channels);
				fprintf(stderr, "  samplerate: %lu\n", hInfo.samplerate);
				fprintf(stderr, "  sbr: %u\n", hInfo.sbr);
				fprintf(stderr, "  object_type: %u\n", hInfo.object_type);
				fprintf(stderr, "  header_type: %u\n", hInfo.header_type);
				fprintf(stderr, "  num_front_channels: %u\n", hInfo.num_front_channels);
				fprintf(stderr, "  num_side_channels: %u\n", hInfo.num_side_channels);
				fprintf(stderr, "  num_back_channels: %u\n", hInfo.num_back_channels);
				fprintf(stderr, "  num_lfe_channels: %u\n", hInfo.num_lfe_channels);
				fprintf(stderr, "  ps: %u\n", hInfo.ps);
				fprintf(stderr, "\n");
				*/

			
				curIndex += hInfo.bytesconsumed;
				//count -= hInfo.bytesconsumed;
				//fprintf(stderr, "%zd %zd\n", curIndex, count);

				//auto readBytes = hInfo.samples * hInfo.channels;//*2;

				auto readBytes = hInfo.samples * 2; //16 is bits per sample for PCM WAV aka 2 bytes per sample

				if (readBytes != 0)
				{

					std::memcpy(reinterpret_cast<char*>(&pcm_all[0])+ pcmSize, output, readBytes);

				}

				pcmSize += readBytes;

			}
			else if (hInfo.error != 0) {
				// Some error occurred while decoding this frame
				fprintf(stderr, "NeAACDecode error: %d\n", hInfo.error);
				fprintf(stderr, "%s\n", NeAACDecGetErrorMessage(hInfo.error));

				/*
				if ((hInfo.error == 15) || (hInfo.error == 13))
				{
					processedAll = true;

					std::vector<short> tempBuf;
					tempBuf.resize(count);

					std::memcpy(reinterpret_cast<char*>(&tempBuf[0]), reinterpret_cast<char*>(&buffer[0]) + curIndex, count);

					std::memcpy(reinterpret_cast<char*>(&buffer[0]), reinterpret_cast<char*>(&tempBuf[0]), count);

					bufferStartPos = count;
				}
				else
				{
					pcmSize = 0;
					return 0;
				}*/

				pcmSize = 0;
				return 0;



			}
			else {
				//fprintf(stderr, "got nothing...\n");
			}

		}

		actualDurationRead = duration;

		std::memcpy(Buffer, &pcm_all[0], pcmSize);

		//pcmShift += pcmSize;

		return pcmSize;

	}

	void AacDecoder::close()
	{

	}

	bool AacDecoder::open(const char* fileName)
	{
		// Get the current config
		NeAACDecConfigurationPtr conf = NeAACDecGetCurrentConfiguration(hAac);

		// XXX: If needed change some of the values in conf

		conf->outputFormat = FAAD_FMT_16BIT;

		// Set the new configuration
		NeAACDecSetConfiguration(hAac, conf);

		f.close();

		f.open(fileName, std::ios::binary);

		auto mustReadCount = aacBuffer.size();

		f.read(reinterpret_cast<char*>(&aacBuffer[0]), mustReadCount);

		if (!aacInited)
		{

			auto count = 1024 * 1024;

			char err = NeAACDecInit(hAac, reinterpret_cast<unsigned char*>(&aacBuffer[0]), count, &samplerate, &channels);
			if (err != 0) {
				// Handle error
				//fprintf(stderr, "NeAACDecInit error: %d\n", err);
				return false;
			}

			aacInited = true;
		}

		//innerRead();

		//openMp3Output();

		return true;
	}


}
