#include "WaveDecoder.h"

#include <fstream>

#include <lame_imported/imported.h>

namespace Decoding
{
	void WaveDecoder::close()
	{
		fclose(file);
		file = NULL;
	}

	WaveDecoder::~WaveDecoder()
	{
		if (file)
		{
			close();
		}
	}

	int WaveDecoder::open(const char* fileName)
	{
		if (file)
		{
			close();
		}

		file = fopen(fileName, "rb");

		if (!file)
		{
			return 0;
		}

		size_t nBytesRead = 0;
		size_t nBytesToRead = 0;
		int nBytesToSeek = 0;

		nBytesRead = fread(&(Header.m_cIFFHeader), 1, sizeof(Header.m_cIFFHeader), file);

		if (sizeof(Header.m_cIFFHeader) != nBytesRead)
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

		while (!feof(file))
		{
			nBytesRead = fread(&(tmpHeader), 1, \
				sizeof(WaveFileChunks::ChunkHeader), file);

			if (nBytesRead != sizeof(WaveFileChunks::ChunkHeader))
			{
				close();
				return false;
			}

			if (0 == memcmp(tmpHeader.m_id, Header.m_cFormatChunk.m_id, 4))
			{
				//Format header found;
				break;
			}

			if (0 != fseek(file, tmpHeader.m_nChunkSize, SEEK_CUR))
			{
				close();
				return false;
			}
		}

		if (feof(file))
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

		nBytesRead = fread(&(Header.m_cFormatChunk.m_nFormatTag)
			, 1
			, nBytesToRead
			, file);
		if (nBytesToSeek > 0)
		{
			if (0 != fseek(file, nBytesToSeek, SEEK_CUR))
			{
				close();
				return false;
			}
		}

		if (nBytesRead != nBytesToRead)
		{
			close();
			return false;
		}

		// Chunk size is copied here.
		Header.m_cFormatChunk << tmpHeader;
		Header.m_cFormatChunk.m_nChunkSize = 18;

		if (Header.m_cFormatChunk.isOk() == false)
		{
			close();
			return false;
		}

		while (!feof(file))
		{
			nBytesRead = fread(&(tmpHeader), 1, \
				sizeof(WaveFileChunks::ChunkHeader), file);

			if (nBytesRead != sizeof(WaveFileChunks::ChunkHeader))
			{
				close();
				return false;
			}

			if (0 == memcmp(tmpHeader.m_id, Header.m_cDataChunk.m_id, 4))
			{
				//Data header found;
				break;
			}

			if (0 != fseek(file, tmpHeader.m_nChunkSize, SEEK_CUR))
			{
				close();
				return false;
			}
		}

		Header.m_cDataChunk << tmpHeader;


		if (feof(file) && (0 != Header.m_cDataChunk.m_id))
		{
			close();
			return false;
		}

		return 1;
	}


	int WaveDecoder::read(char* Buffer, size_t Count)
	{
		int count = fread(Buffer, 1, Count, file);

		if (count < 1)
		{
			close();
		}

		return count;
	}
}
