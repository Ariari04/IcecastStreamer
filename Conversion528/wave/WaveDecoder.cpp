#include "WaveDecoder.h"

#include <fstream>

#include "lame.h"

#include <iostream>
#include <vector>
#include <cstring>

namespace Decoding
{

	void WaveToMp3Decoder::close()
	{
		f.close();
		//fclose(file);
		//file = NULL;
	}

	WaveToMp3Decoder::~WaveToMp3Decoder()
	{
		f.close();
	}

	int WaveToMp3Decoder::open(const char* fileName)
	{
		f.close();

		//file = fopen(fileName, "rb");
		f.open(fileName, std::ios::binary);

		if (!f)
		{
			return 0;
		}

		size_t nBytesRead = 0;
		size_t nBytesToRead = 0;
		int nBytesToSeek = 0;

		//nBytesRead = fread(&(Header.m_cIFFHeader), 1, sizeof(Header.m_cIFFHeader), file);

		f.read(reinterpret_cast<char*>(&(Header.m_cIFFHeader)), sizeof(Header.m_cIFFHeader));

		if (!f)
		{
			Header.m_cIFFHeader = WaveFileChunks::IFFHeader();
			close();
			return 0;
		}

		if (!Header.m_cIFFHeader.isOk())
		{
			Header.m_cIFFHeader = WaveFileChunks::IFFHeader();
			close();
			return false;
		}


		WaveFileChunks::ChunkHeader tmpHeader;

		//while (!feof(file))
		while (!f.eof())
		{
			/*
			nBytesRead = fread(&(tmpHeader), 1, \
				sizeof(WaveFileChunks::ChunkHeader), file);

			if (nBytesRead != sizeof(WaveFileChunks::ChunkHeader))
			{
				close();
				return false;
			}*/

			f.read(reinterpret_cast<char*>(&(tmpHeader)), sizeof(WaveFileChunks::ChunkHeader));

			if (!f)
			{
				break;
			}


			if (0 == memcmp(tmpHeader.m_id, Header.m_cFormatChunk.m_id, 4))
			{
				//Format header found;
				break;
			}


			/*
			if (0 != fseek(file, tmpHeader.m_nChunkSize, SEEK_CUR))
			{
				close();
				return false;
			}*/


			f.seekg(tmpHeader.m_nChunkSize, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}


		}

		//if (feof(file))
		if (f.eof())
		{
			close();
			return false;
		}

		if (tmpHeader.m_nChunkSize < 16)
		{
			close();
			return false;
		}


		nBytesToSeek = tmpHeader.m_nChunkSize - (sizeof(Header.m_cFormatChunk) - 8);
		nBytesToRead =
			(nBytesToSeek > 0)
			?
			sizeof(Header.m_cFormatChunk) - 8
			:
			tmpHeader.m_nChunkSize;

		/*
		nBytesRead = fread(&(Header.m_cFormatChunk.m_nFormatTag)
			, 1
			, nBytesToRead
			, file);*/

		f.read(reinterpret_cast<char*>(&(Header.m_cFormatChunk.m_nFormatTag)), nBytesToRead);

		if (!f)
		{
			close();
			return false;
		}


		if (nBytesToSeek > 0)
		{
			/*
			if (0 != fseek(file, nBytesToSeek, SEEK_CUR))
			{
				close();
				return false;
			}*/


			f.seekg(nBytesToSeek, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}



		}
		/*
		if (nBytesRead != nBytesToRead)
		{
			close();
			return false;
		}*/

		// Chunk size is copied here.
		Header.m_cFormatChunk << tmpHeader;
		Header.m_cFormatChunk.m_nChunkSize = 18;

		if (Header.m_cFormatChunk.isOk() == false)
		{
			close();
			return false;
		}

		//while (!feof(file))
		while (!f.eof())
		{
			/*
			nBytesRead = fread(&(tmpHeader), 1, \
				sizeof(WaveFileChunks::ChunkHeader), file);

			if (nBytesRead != sizeof(WaveFileChunks::ChunkHeader))
			{
				close();
				return false;
			}*/

			f.read(reinterpret_cast<char*>(&tmpHeader), sizeof(WaveFileChunks::ChunkHeader));

			if (!f)
			{
				close();
				return false;
			}

			if (0 == memcmp(tmpHeader.m_id, Header.m_cDataChunk.m_id, 4))
			{
				//Data header found;
				break;
			}

			/*
			if (0 != fseek(file, tmpHeader.m_nChunkSize, SEEK_CUR))
			{
				close();
				return false;
			}*/

			f.seekg(tmpHeader.m_nChunkSize, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}
		}

		Header.m_cDataChunk << tmpHeader;


		//if (feof(file) && (0 != Header.m_cDataChunk.m_id))
		if (f.eof() && (0 != Header.m_cDataChunk.m_id))
		{
			close();
			return false;
		}


		std::cout << "f.tellg() after open = " << f.tellg() << std::endl;

		openMp3Output();

		return 1;
	}


	int WaveToMp3Decoder::openMp3Output()
	{
		lame = lame_init();
		lame_set_in_samplerate(lame, this->Header.m_cFormatChunk.m_nSamplesPerSec);
		lame_set_VBR(lame, vbr_default);
		lame_init_params(lame);

		return 1;
	}

	int WaveToMp3Decoder::readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration)
	{
		if (f.eof())
		{
			if (!flush_sent)
			{
				flush_sent = true;
				return lame_encode_flush(lame, reinterpret_cast<unsigned char*>(Buffer), Count);
			}
			else
			{
				close();
				return 0;
			}
			
		}

		auto readCount = (this->Header.m_cFormatChunk.m_nChannels * this->Header.m_cFormatChunk.m_nSamplesPerSec * this->Header.m_cFormatChunk.m_nBitsPerSample * duration.count() / 1000) / 8;
		
		std::vector<short> buf;
		buf.resize(readCount / sizeof(short));

		//std::cout << "readCount interleaved: " << readCount << std::endl;

		std::cout << "f.tellg() before = " << f.tellg() << std::endl;

		f.read(reinterpret_cast<char*>(&buf[0]), readCount);

		std::cout << "f.tellg() after = " << f.tellg() << std::endl;

		//int write = lame_encode_buffer_interleaved(lame, &buf[0], this->Header.m_cFormatChunk.m_nSamplesPerSec, reinterpret_cast<unsigned char*>(Buffer), Count);

		int write = lame_encode_buffer_interleaved(lame, &buf[0], buf.size()/ this->Header.m_cFormatChunk.m_nChannels, reinterpret_cast<unsigned char*>(Buffer), Count);

		return write;
		
	}
}
