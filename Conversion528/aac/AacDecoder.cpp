#include <aac/AacDecoder.h>

#include <string.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <faad_imported/main_imported.h>

namespace Decoding
{
	AacDecoder::AacDecoder()
	{
	}

	AacDecoder::~AacDecoder()
	{
	}

	int AacDecoder::open(const char* fileName)
	{
		int argc = 3;
		char* argv[3] = { "", "-f 2", (char*)fileName };

		int ret = faad_open_decoding(argc, argv, &mp4SampleCount);

		firstZero = true;

		return 1 - ret;
	}

	int AacDecoder::read(char* Buffer, size_t Count)
	{
		int iread = faad_iteration_decoding(Buffer, Count);

		if (firstZero && iread == 0)
		{
			iread = faad_iteration_decoding(Buffer, Count);
		}

		if (iread < 1)
		{
			faad_close_decoding();
			return -1;
		}

		firstZero = false;

		return iread;
	}



	//---------------------------------------


	AacToMp3Decoder::AacToMp3Decoder()
	{
		
	}

	AacToMp3Decoder::~AacToMp3Decoder()
	{
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

		if (!f)
		{
			return 0;
		}

		f.read(reinterpret_cast<char*>(&input[0]), input.size());

		input_size = f.gcount();

		// read the first buffer... will be used in `NeAACDecInit()` call...
		//input_size = fread(input, 1, sizeof input, stdin);
		//fprintf(stderr, "read %d bytes\n", (int)input_size);

		// Initialize the library using one of the initialization functions
		int done = 0;
		
		char err = NeAACDecInit(hAac, &input[0], input_size, &samplerate, &channels);
		if (err != 0) {
			// Handle error
			fprintf(stderr, "NeAACDecInit error: %d\n", err);
			return 0;
		}
		fprintf(stderr, "{ samplerate: %lu, channels: %u, bytesRead: %d }\n", samplerate, channels, err);


		openMp3Output();

		return 1;
	}

	int AacToMp3Decoder::read(char* Buffer, size_t Count)
	{
		return 0;

		/*
		int iread = faad_iteration_decoding(Buffer, Count);

		if (firstZero && iread == 0)
		{
			iread = faad_iteration_decoding(Buffer, Count);
		}

		if (iread < 1)
		{
			faad_close_decoding();
			return -1;
		}

		firstZero = false;

		return iread;*/
	}

	int AacToMp3Decoder::readDuration(char* Buffer, size_t Count, std::chrono::seconds duration)
	{
		
		auto readCount = duration.count() * samplerate*channels*2;

		//static bool once = false;

		NeAACDecFrameInfo hInfo;
		memset(&hInfo, 0, sizeof hInfo);

		//if (!once)
		//{
			do {

				auto output = NeAACDecDecode(hAac, &hInfo, &input[curIndex], input_size);

				if ((hInfo.error == 0) && (hInfo.samples > 0)) {
					// do what you need to do with the decoded samples
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

				}
				else if (hInfo.error != 0) {
					// Some error occurred while decoding this frame
					fprintf(stderr, "NeAACDecode error: %d\n", hInfo.error);
					fprintf(stderr, "%s\n", NeAACDecGetErrorMessage(hInfo.error));
					return 0;
				}
				else {
					fprintf(stderr, "got nothing...\n");
				}

				curIndex += hInfo.bytesconsumed;
				input_size -= hInfo.bytesconsumed;
				fprintf(stderr, "%zd %zd\n", curIndex, input_size);

				auto readBytes = hInfo.samples* hInfo.channels;

				if (readBytes != 0)
				{

					//std::memcpy(&pcmBuffer[totalRead], output, readBytes);
					std::memcpy(reinterpret_cast<char*>(&pcmBuffer[0]) + totalRead, output, readBytes);

				}

				totalRead += readBytes;

			} while (totalRead < readCount);

			//once = true;
		//}

		/*
		std::vector<short> pcm_r;

		pcm_r.resize(readCount*2);

		std::vector<short> pcm_l;

		pcm_l.resize(readCount * 2);

		std::memset(&pcm_l[0], 0, readCount * 2);
		
		for (size_t i = 0; i < readCount/2; i++)
		{
			pcm_l[i] = pcmBuffer[2*i];
		}

		std::memset(&pcm_r[0], 0, readCount*2);*/

	    int write = lame_encode_buffer_interleaved(lame, &pcmBuffer[0], readCount/(channels*2), reinterpret_cast<unsigned char*>(Buffer), Count);
		//int write = lame_encode_buffer(lame, &pcm_l[0], &pcm_r[0], readCount, reinterpret_cast<unsigned char*>(Buffer), Count);


		
		auto leftoverCount = totalRead - readCount;

		if (leftoverCount > 0)
		{

			std::vector<short> tempBuf;
			tempBuf.resize(leftoverCount);

			std::memcpy(reinterpret_cast<char*>(&tempBuf[0]), reinterpret_cast<char*>(&pcmBuffer[0]) + readCount, leftoverCount * sizeof(short));

			std::memcpy(reinterpret_cast<char*>(&pcmBuffer[0]), reinterpret_cast<char*>(&tempBuf[0]), leftoverCount * sizeof(short));

		}

		totalRead = leftoverCount;
		

		return write;



		/*

		do
		{

			int iread = faad_iteration_decoding(reinterpret_cast<char*>(&pcmBuffer[0]), readCount- totalRead);

			if (firstZero && iread == 0)
			{
				iread = faad_iteration_decoding(reinterpret_cast<char*>(&pcmBuffer[0]), readCount - totalRead);

				firstZero = false;
			}

			if (iread < 1)
			{
				faad_close_decoding();
				return -1;
			}

			totalRead += iread;

		} while (totalRead < readCount);

		int write = lame_encode_buffer_interleaved(lame, &pcmBuffer[0], readCount, reinterpret_cast<unsigned char*>(Buffer), Count);

		return write;

		

		//return iread;*/

		return 0;

	}


	int AacToMp3Decoder::openMp3Output()
	{
		lame = lame_init();
		lame_set_in_samplerate(lame, samplerate);
		lame_set_VBR(lame, vbr_default);
		lame_init_params(lame);

		return 1;
	}

}
