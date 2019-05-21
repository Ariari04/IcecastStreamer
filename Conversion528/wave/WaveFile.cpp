#define _CRT_SECURE_NO_DEPRECATE
#include "WaveFile.h"
#include <stdio.h>
#include <string.h>
#include <cassert>

void WaveFileChunks::ChunkHeader::operator << (const ChunkHeader& rhs)
{
	memcpy(this, &rhs, sizeof(ChunkHeader));
}

WaveFileChunks::IFFHeader::IFFHeader()
{
	m_id[0] =		'R';
	m_id[1] =		'I';
	m_id[2] =		'F';
	m_id[3] =		'F';

	m_nChunkSize = 0;

	m_idFileFormat[0] = 'W';
	m_idFileFormat[1] = 'A';
	m_idFileFormat[2] = 'V';
	m_idFileFormat[3] = 'E';
}

bool WaveFileChunks::IFFHeader::isOk() const
{
	IFFHeader tmpHeader;
	tmpHeader.m_nChunkSize = m_nChunkSize;
	return 0 == memcmp(this, &tmpHeader, sizeof(IFFHeader));
}

WaveFileChunks::FormatChunk::FormatChunk()
{
	m_id[0]	=	'f';
	m_id[1]	=	'm';
	m_id[2]	=	't';
	m_id[3]	=	' ';

	m_nBitsPerSample = 16; // 16 bits per sample.

	m_nChunkSize = sizeof(FormatChunk) - sizeof(m_id) - sizeof(m_nChunkSize);

	m_nFormatTag = 1; // 1 means no compression.

	m_nSamplesPerSec = 44100; // 44.1 kHz quality.

	m_nChannels = 2; // stereo.

	//Size of one sample in bytes.
	m_nBlockAlign = m_nChannels * (m_nBitsPerSample / 8);

	//Average bytes to store one second of sound.
	m_nAvgBytesPerSec = m_nSamplesPerSec * m_nBlockAlign;

	m_nAddParamsSize = 0;
}


void WaveFileChunks::FormatChunk::updateASRAndBA()
{
	m_nAvgBytesPerSec = (m_nSamplesPerSec*m_nChannels*(m_nBitsPerSample/8));
	m_nBlockAlign = m_nChannels*m_nBitsPerSample/8;
}
bool WaveFileChunks::FormatChunk::setBitsPerSample(uint16_t nBitsPerSample)
{
	FormatChunk tmpChunk(*this);
	tmpChunk.m_nBitsPerSample = nBitsPerSample;
	tmpChunk.updateASRAndBA();
	if(tmpChunk.isOk())
	{
		m_nBitsPerSample = nBitsPerSample;
		updateASRAndBA();
		return true;
	}
	else
	{
		return false;
	}
}
bool WaveFileChunks::FormatChunk::setNumberOfChannels(uint16_t nChannels)
{
	FormatChunk tmpChunk(*this);
	tmpChunk.m_nChannels = nChannels;
	tmpChunk.updateASRAndBA();
	if(tmpChunk.isOk())
	{
		m_nChannels = nChannels;
		updateASRAndBA();
		return true;
	}
	else
	{
		return false;
	}
}
bool WaveFileChunks::FormatChunk::setSampleRate(uint32_t nSamplesPerSec)
{
	FormatChunk tmpChunk(*this);
	tmpChunk.m_nSamplesPerSec = nSamplesPerSec;
	tmpChunk.updateASRAndBA();
	if(tmpChunk.isOk())
	{
		m_nSamplesPerSec = nSamplesPerSec;
		updateASRAndBA();
		return true;
	}
	else
	{
		return false;
	}
}
bool WaveFileChunks::FormatChunk::isOk() const
{
	if(0 != memcmp(m_id, "fmt ", 4) )
	{
		return false;
	}

	if(m_nChunkSize != 18)
	{
		return false;
	}

	//ADPCM == 17 and others are not supported.

	if(m_nFormatTag != 1)
	{
		return false;
	}


	/*
	// This is checked in conversion algorithm
	// Moved from here for generality.

	//only stereo is supported
	if(m_nChannels != 2)
	{
		return false;
	}

	//only 44.1 and 48 khz are supported.
	if(!(m_nSamplesPerSec == 44100 || m_nSamplesPerSec == 48000 || m_nSamplesPerSec == 96000))
	{
		return false;
	}

	//Only such bit depthes are supported.
	if(! (m_nBitsPerSample == 8 || m_nBitsPerSample == 16 || m_nBitsPerSample == 32 || m_nBitsPerSample == 64))
	{
		return false;
	}
	*/

	//ByteRate == SampleRate * NumChannels * BitsPerSample/8
	if(m_nAvgBytesPerSec != (m_nSamplesPerSec*m_nChannels*(m_nBitsPerSample/8)))
	{
		return false;
	}

	// BlockAlign == NumChannels * BitsPerSample/8
	if(m_nBlockAlign != m_nChannels*m_nBitsPerSample/8)
	{
		return false;
	}

	if(m_nAddParamsSize != 0)
	{
		return false;
	}

	return true;
}

WaveFileChunks::DataChunk::DataChunk()
{
	m_id[0] =			'd';
	m_id[1] =			'a';
	m_id[2] =			't';
	m_id[3] =			'a';
	m_nChunkSize =		0;
}

bool WaveFileChunks::DataChunk::isOk() const
{
	return (0 == memcmp(m_id, "data", 4));
}
WaveFileChunks::WaveFileHeader::WaveFileHeader()
{
	setSoundSizeInBytes(0);
}

void  WaveFileChunks::WaveFileHeader::setSoundSizeInBytes(uint32_t nSizeOfSoundInBytes)
{
	m_cDataChunk.m_nChunkSize = nSizeOfSoundInBytes;
	m_cIFFHeader.m_nChunkSize = sizeof(WaveFileHeader)	\
			+ nSizeOfSoundInBytes						\
			- sizeof(m_cIFFHeader.m_id)					\
			- sizeof(m_cIFFHeader.m_nChunkSize);
}


uint32_t WaveFileChunks::WaveFileHeader::getSoundSizeInBytes() const
{
	return m_cDataChunk.m_nChunkSize;
}

#undef _CRT_SECURE_NO_DEPRECATE
