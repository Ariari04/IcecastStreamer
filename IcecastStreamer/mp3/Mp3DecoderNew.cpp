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
	//std::array<char, 1024*1024> buffer;

	//std::array<short, BLOCK_SIZE * 64> pcm_l;
	//std::array<short, BLOCK_SIZE * 64> pcm_r;

	//std::array<short, BLOCK_SIZE * 64> tempBuf;

	constexpr int FILE_BUFFER_SIZE = 1024 * 32; //Must be good enough to fit at least 10 mp3 frames, so 16kb


	MP3DHolder Mp3WaveMp3DecoderNew::mp3dHolder;
	
	Mp3WaveMp3DecoderNew::Mp3WaveMp3DecoderNew()
	{
		//lameInput = hip_decode_init();

		capacityPcmBuffer.reserve(200000); //176400 is 1s, need a little bit more
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

		fileBuffer.resize(FILE_BUFFER_SIZE);

		f.read(reinterpret_cast<char*>(&fileBuffer[0]), FILE_BUFFER_SIZE);

		auto count = f.gcount();

		if (count != FILE_BUFFER_SIZE)
		{
			close();
			return 0;
		}
		
		return true;
	}


	int Mp3WaveMp3DecoderNew::readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration, std::chrono::milliseconds& actualDurationRead)
	{


		if (fileIsCompletelyOver)
		{
			actualDurationRead = std::chrono::milliseconds(0);
			return 0;
		}

		mp3dec_frame_info_t info;
		static short pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
		/*unsigned char *input_buf; - input byte stream*/


		int nChannels = 2;

		int nSamplesPerSec = 44100;

		int nBitsPerSample = 16;

		int maxCount = (nChannels * nSamplesPerSec * nBitsPerSample * duration.count() / 1000) / 8;

		int readPcmBytes = leftoverSize;
	

		//fileIsOver = f.eof();


		//bool fileIsOver = false;


		bool audioDataFound = false;

		int samples = 0;

		while ((readPcmBytes < maxCount) && !fileIsCompletelyOver)
		{
			audioDataFound = false;

			while (!audioDataFound && !fileIsCompletelyOver)
			{
				if (bufferPos < fileBuffer.size())
				{
					samples = mp3dec_decode_frame(&mp3dHolder.mp3d, reinterpret_cast<uint8_t*>(&fileBuffer[bufferPos]), fileBuffer.size() - bufferPos, pcm, &info);
				}
				else
				{
					fileIsCompletelyOver = true;
					//samples = 0;
					//info.frame_bytes = 0;
				}

				if (samples > 0)
				{
					audioDataFound = true;
				}
				else if (samples == 0)
				{
					if (info.frame_bytes == 0)
					{
						//audioDataFound = true;
						fileIsCompletelyOver = true;
					}
					/*
					if (info.frame_bytes == 0)
					{
						int localLeftoverSize = fileBuffer.size() - bufferPos;

						for (int i = 0; i < localLeftoverSize; i++)
						{
							fileBuffer[0 + i] = fileBuffer[bufferPos + i];
						}
						//capacityPcmBuffer.resize(leftoverSize);

						bufferPos = 0;
						//Insufficient data, decode more
						f.read(reinterpret_cast<char*>(&fileBuffer[localLeftoverSize]), FILE_BUFFER_SIZE - localLeftoverSize);

						auto count = f.gcount();

						if (f.eof())
						{
							close();
							return 0;
						}

						if (count != FILE_BUFFER_SIZE - localLeftoverSize)
						{
							close();
							return 0;
						}
					}	*/				
				}


				if (!fileIsCompletelyOver)
				{
					bufferPos += info.frame_bytes;


					if (!fileIsOver && (bufferPos > FILE_BUFFER_SIZE / 2)) //More than half data consumed, need to load another part
					{
						int localLeftoverSize = fileBuffer.size() - bufferPos;

						for (int i = 0; i < localLeftoverSize; i++)
						{
							fileBuffer[0 + i] = fileBuffer[bufferPos + i];
						}

						f.read(reinterpret_cast<char*>(&fileBuffer[localLeftoverSize]), FILE_BUFFER_SIZE - localLeftoverSize);

						auto count = f.gcount();

						if (count != FILE_BUFFER_SIZE - localLeftoverSize)
						{
							fileBuffer.resize(localLeftoverSize + count);
							fileIsOver = true;
							close();
							//return 0;
						}

						bufferPos = 0;
					}


					int shift = samples * info.channels * 2;

					readPcmBytes += shift;

					capacityPcmBuffer.insert(capacityPcmBuffer.end(), reinterpret_cast<char*>(pcm), reinterpret_cast<char*>(pcm) + shift);
				}
			}

		}




		int write = std::min(readPcmBytes, maxCount); //Don't send more than requested

		memcpy(Buffer, &capacityPcmBuffer[0], write);

		actualDurationRead = duration * write / maxCount;

		leftoverSize = std::max(readPcmBytes - maxCount, 0); //No less than 0

		//If there is a little more, we move it to the beginning of the buffer:
		if (readPcmBytes > maxCount)
		{

			for (int i = 0; i < leftoverSize; i++)
			{
				capacityPcmBuffer[0 + i] = capacityPcmBuffer[maxCount + i];
			}
			
		}

		capacityPcmBuffer.resize(leftoverSize);

		return write;

	}
}
