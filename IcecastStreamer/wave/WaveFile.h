/* Copyright Zaurmann Software 2010 */
#ifndef _MY_WAVE_FILE_H_
#define _MY_WAVE_FILE_H_

//#include "StaticAssertion.h"
//#include "IntegerTypes.h"
//#include "KString.h"

#include <AudioFile.h>

namespace WaveFileChunks
{

	#pragma pack(push,1)

	typedef uint8_t ChunkID[4];

	static_assert(sizeof(ChunkID) == 4, "sizeof(ChunkID) == 4");


	struct ChunkHeader
	{
		ChunkID				m_id;
		uint32_t			m_nChunkSize;

		// not = to comfortable substitution in subclasses

		void operator << (const ChunkHeader& rhs);
	};

	static_assert(sizeof(ChunkHeader) == 8, "sizeof(ChunkHeader) == 8");

	struct IFFHeader : public ChunkHeader
	{
		//"WAVE"
		ChunkID m_idFileFormat;

		IFFHeader();

		bool isOk() const;
	};

	static_assert(sizeof(IFFHeader) == 12, "sizeof(IFFHeader) == 12");

	struct FormatChunk : public ChunkHeader
	{
		uint16_t	m_nFormatTag;
		uint16_t	m_nChannels;
		uint32_t	m_nSamplesPerSec;
		uint32_t	m_nAvgBytesPerSec;
		uint16_t	m_nBlockAlign;
		uint16_t	m_nBitsPerSample;
		uint16_t	m_nAddParamsSize;

		FormatChunk();
		bool isOk() const;
		void updateASRAndBA();
		bool setBitsPerSample(uint16_t nBitsPerSample);
		bool setNumberOfChannels(uint16_t nChannels);
		bool setSampleRate(uint32_t nSamplesPerSec);
	};


	static_assert(sizeof(FormatChunk) == 26, "sizeof(FormatChunk) == 26");

	struct DataChunk : public ChunkHeader
	{
		DataChunk();
		bool isOk() const;
	};

	static_assert(sizeof(DataChunk) == 8, "sizeof(DataChunk) == 8");

	struct WaveFileHeader
	{
		IFFHeader		m_cIFFHeader;
		FormatChunk		m_cFormatChunk;
		DataChunk		m_cDataChunk;
		WaveFileHeader();

		void setSoundSizeInBytes(uint32_t nSizeOfSoundInBytes);
		uint32_t getSoundSizeInBytes() const;

	};

	static_assert(sizeof(WaveFileHeader) == 46, "sizeof(WaveFileHeader) == 46");

	#pragma pack(pop)
}


#endif
