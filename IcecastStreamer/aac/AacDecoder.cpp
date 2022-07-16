#include <aac/AacDecoder.h>

#include <string.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace Decoding
{


	AacDecoder::~AacDecoder()
	{
		close();
	}

	int AacDecoder::readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration, std::chrono::milliseconds& actualDurationRead)
	{
	
		pcmSize = 0;

		//int secondSize = 176400;

		int nChannels = 2;

		int nSamplesPerSec = 44100;

		int nBitsPerSample = 16;

		int maxCount = (nChannels * nSamplesPerSec * nBitsPerSample * duration.count() / 1000) / 8;

		while (pcmSize < maxCount)
		{
			auto count = inputFileBufferSize - curIndex;

			if (count == 0)
			{
				break;
			}

			NeAACDecFrameInfo hInfo;
			memset(&hInfo, 0, sizeof hInfo);

			auto output = NeAACDecDecode(hAac, &hInfo, reinterpret_cast<unsigned char*>(&inputFileBuffer[curIndex]), count);

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

			if (fileIsOver)
			{
				if (curIndex == inputFileBufferSize)
				{
					break;
				}
			}
			else
			{
				int newCount = inputFileBufferSize - curIndex;

				if (newCount < 1024)
				{
					

					//Move the data from right back to left
					memcpy(&inputFileBuffer[0], &inputFileBuffer[curIndex], newCount);

					int requestedAmountFromFile = inputFileBuffer.size() - newCount;

					f.read(reinterpret_cast<char*>(&inputFileBuffer[newCount]), requestedAmountFromFile);

					auto gcount = f.gcount();
					inputFileBufferSize = gcount + newCount;

					if (gcount < requestedAmountFromFile)
					{
						//fileBuffer.resize(localLeftoverSize + count);
						//inputFileBufferSize = gcount;
						fileIsOver = true;
						close();
						//return 0;
					}

					//Refill the file
					curIndex = 0;
				}
			}

		}

		//actualDurationRead = duration;
		actualDurationRead = duration * pcmSize / maxCount;

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

		f.read(reinterpret_cast<char*>(&inputFileBuffer[0]), inputFileBuffer.size());

		auto count = f.gcount();

		inputFileBufferSize = count;

		if (count != inputFileBuffer.size())
		{
			fileIsOver = true;
			close();
		}

		if (!aacInited)
		{

			auto count = inputFileBuffer.size();

			char err = NeAACDecInit(hAac, reinterpret_cast<unsigned char*>(&inputFileBuffer[0]), count, &samplerate, &channels);
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
