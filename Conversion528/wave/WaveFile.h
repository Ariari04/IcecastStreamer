/* Copyright Zaurmann Software 2010 */
#ifndef _MY_WAVE_FILE_H_
#define _MY_WAVE_FILE_H_

#include "StaticAssertion.h"
#include "IntegerTypes.h"
#include "KString.h"

#include <AudioFile.h>

namespace WaveFileChunks
{

	#pragma pack(push,1)

	typedef uint8 ChunkID[4];

	CheckSize(ChunkID, 4);

	struct ChunkHeader
	{
		ChunkID				m_id;
		uint32			m_nChunkSize;

		// not = to comfortable substitution in subclasses

		void operator << (const ChunkHeader& rhs);
	};

	CheckSize(ChunkHeader, 8);

	struct IFFHeader : public ChunkHeader
	{
		//"WAVE"
		ChunkID m_idFileFormat;

		IFFHeader();

		bool isOk() const;
	};

	CheckSize(IFFHeader, 12);

	struct FormatChunk : public ChunkHeader
	{
		uint16	m_nFormatTag;
		uint16	m_nChannels;
		uint32	m_nSamplesPerSec;
		uint32	m_nAvgBytesPerSec;
		uint16	m_nBlockAlign;
		uint16	m_nBitsPerSample;
		uint16	m_nAddParamsSize;

		FormatChunk();
		bool isOk() const;
		void updateASRAndBA();
		bool setBitsPerSample(uint16 nBitsPerSample);
		bool setNumberOfChannels(uint16 nChannels);
		bool setSampleRate(uint32 nSamplesPerSec);
	};

	CheckSize(FormatChunk, 26);

	struct DataChunk : public ChunkHeader
	{
		DataChunk();
		bool isOk() const;
	};

	CheckSize(DataChunk, 8);

	struct WaveFileHeader
	{
		IFFHeader		m_cIFFHeader;
		FormatChunk		m_cFormatChunk;
		DataChunk		m_cDataChunk;
		WaveFileHeader();

		void setSoundSizeInBytes(uint32 nSizeOfSoundInBytes);
		uint32 getSoundSizeInBytes() const;

	};

	CheckSize(WaveFileHeader, 46);

	#pragma pack(pop)
}


#endif
