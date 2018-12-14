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
bool WaveFileChunks::FormatChunk::setBitsPerSample(uint16 nBitsPerSample)
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
bool WaveFileChunks::FormatChunk::setNumberOfChannels(uint16 nChannels)
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
bool WaveFileChunks::FormatChunk::setSampleRate(uint32 nSamplesPerSec)
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

void  WaveFileChunks::WaveFileHeader::setSoundSizeInBytes(uint32 nSizeOfSoundInBytes)
{
	m_cDataChunk.m_nChunkSize = nSizeOfSoundInBytes;
	m_cIFFHeader.m_nChunkSize = sizeof(WaveFileHeader)	\
			+ nSizeOfSoundInBytes						\
			- sizeof(m_cIFFHeader.m_id)					\
			- sizeof(m_cIFFHeader.m_nChunkSize);
}


uint32 WaveFileChunks::WaveFileHeader::getSoundSizeInBytes() const
{
	return m_cDataChunk.m_nChunkSize;
}
WaveFile::WaveFileReader::WaveFileReader()
: m_hFile(0)
,m_nLastError(WFREOk)
{
}
#ifdef WIN32
WaveFile::WaveFileReader::WaveFileReader(const wchar_t* strFileName)
: m_hFile(0)
,m_nLastError(WFREOk)
{
	open(strFileName);
}
#endif

WaveFile::WaveFileReader::WaveFileReader(const char* strFileName)
: m_hFile(0)
,m_nLastError(WFREOk)
{
	open(strFileName);
}

WaveFile::WaveFileReaderError WaveFile::WaveFileReader::getLastError() const
{
	return m_nLastError;
}

#ifdef WIN32
bool WaveFile::WaveFileReader::open(const wchar_t* strFileName)
{
	if(isOpened())
	{
		m_nLastError = WFREAlreadyOpened;
		return false;
	}

	m_hFile = _wfopen(strFileName, L"rb");

	if(m_hFile == 0)
	{
		m_nLastError = WFRECanNotOpenFile;
		return false;
	}

	return openImpl();
}
#endif

bool WaveFile::WaveFileReader::open(const char* strFileName)
{
	if(isOpened())
	{
		m_nLastError = WFREAlreadyOpened;
		return false;
	}

	m_hFile = fopen(strFileName, "rb");

	if(m_hFile == 0)
	{
		m_nLastError = WFRECanNotOpenFile;
		return false;
	}

	return openImpl();
}

bool WaveFile::WaveFileReader::openImpl()
{
    assert(isOpened());

	size_t nBytesRead = 0;
	size_t nBytesToRead = 0;
	int nBytesToSeek = 0;

	nBytesRead = fread(&(m_cHeader.m_cIFFHeader), 1, sizeof(m_cHeader.m_cIFFHeader), m_hFile);

	if(sizeof(m_cHeader.m_cIFFHeader) != nBytesRead)
	{
		m_cHeader.m_cIFFHeader = WaveFileChunks::IFFHeader();
		close();
		m_nLastError = WFRECorruptedWaveFile;
		return false;
	}

	if(!m_cHeader.m_cIFFHeader.isOk())
	{
		m_cHeader.m_cIFFHeader = WaveFileChunks::IFFHeader();
		close();
		m_nLastError = WFRECorruptedWaveFile;
		return false;
	}


	WaveFileChunks::ChunkHeader tmpHeader;

	while(!feof(m_hFile))
	{
		int i = 0;
		++i;
		if(i>100)
		{
			close();
			return false;
		}

		nBytesRead = fread(&(tmpHeader), 1, \
			sizeof(WaveFileChunks::ChunkHeader), m_hFile);

		if(nBytesRead != sizeof(WaveFileChunks::ChunkHeader))
		{
			close();
			m_nLastError = WFRECorruptedWaveFile;
			return false;
		}

		if(0 == memcmp(tmpHeader.m_id, m_cHeader.m_cFormatChunk.m_id, 4))
		{
			//Format header found;
			break;
		}

		if(0 != fseek(m_hFile, tmpHeader.m_nChunkSize, SEEK_CUR))
		{
			close();
			m_nLastError = WFRECorruptedWaveFile;
			return false;
		}
	}

	if(feof(m_hFile))
	{
		close();
		m_nLastError = WFRECorruptedWaveFile;
		return false;
	}

	if(tmpHeader.m_nChunkSize < 16)
	{
		close();
		m_nLastError = WFRECorruptedWaveFile;
		return false;
	}


	nBytesToSeek = tmpHeader.m_nChunkSize - (sizeof(m_cHeader.m_cFormatChunk) - 8);
	nBytesToRead =
		(nBytesToSeek > 0)
				?
				sizeof(m_cHeader.m_cFormatChunk) - 8
				:
				tmpHeader.m_nChunkSize;

	nBytesRead = fread(&(m_cHeader.m_cFormatChunk.m_nFormatTag)
			, 1
			, nBytesToRead
			, m_hFile);
	if(nBytesToSeek > 0 )
	{
		if(0 != fseek(m_hFile, nBytesToSeek, SEEK_CUR))
		{
			close();
			m_nLastError = WFRECorruptedWaveFile;
			return false;
		}
	}

	if(nBytesRead != nBytesToRead)
	{
		close();
		m_nLastError = WFRECorruptedWaveFile;
		return false;
	}

	// Chunk size is copied here.
	m_cHeader.m_cFormatChunk << tmpHeader;
	m_cHeader.m_cFormatChunk.m_nChunkSize = 18;

	if(m_cHeader.m_cFormatChunk.isOk() == false)
	{
		close();
		m_nLastError = WFREUnsupportedFormat;
		return false;
	}

	while(!feof(m_hFile))
	{
		int i = 0;
		++i;
		if(i>100)
		{
			close();
			m_nLastError = WFRECorruptedWaveFile;
			return false;
		}

		nBytesRead = fread(&(tmpHeader), 1, \
			sizeof(WaveFileChunks::ChunkHeader), m_hFile);

		if(nBytesRead != sizeof(WaveFileChunks::ChunkHeader))
		{
			close();
			m_nLastError = WFRECorruptedWaveFile;
			return false;
		}

		if(0 == memcmp(tmpHeader.m_id, m_cHeader.m_cDataChunk.m_id, 4))
		{
			//Data header found;
			break;
		}

		if(0 != fseek(m_hFile, tmpHeader.m_nChunkSize, SEEK_CUR))
		{
			close();
			m_nLastError = WFRECorruptedWaveFile;
			return false;
		}
	}

	m_cHeader.m_cDataChunk << tmpHeader;


	if(feof(m_hFile) && (0 != m_cHeader.m_cDataChunk.m_id))
	{
		close();
		m_nLastError = WFRECorruptedWaveFile;
		return false;
	}

	m_nLastError = WFREOk;
	return true;
}

bool WaveFile::WaveFileReader::close()
{
	if(isOpened())
	{
		int res = fclose(m_hFile);
		if(res == 0)
		{
			m_hFile = 0;
			m_nLastError = WFREOk;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		m_nLastError = WFREFileNotOpened;
		return false;
	}
}

bool WaveFile::WaveFileReader::isOpened() const
{
	return m_hFile != 0;
}

WaveFile::WaveFileReader::~WaveFileReader()
{
	if(isOpened())
	{
		close();
	}
}

const WaveFileChunks::WaveFileHeader& WaveFile::WaveFileReader::getWaveFileHeader() const
{
	return m_cHeader;
}


size_t WaveFile::WaveFileReader::MusicFread(void* DstBuf, size_t ElementSize, size_t Count)
{
	return fread(DstBuf, ElementSize, Count, m_hFile);
}
int    WaveFile::WaveFileReader::MusicFeof()
{
	return feof(m_hFile);
}

WaveFile::WaveFileWriter::WaveFileWriter()
: m_hFile(0)
, m_bHeaderWritten(false)
{
}

#ifdef WIN32
WaveFile::WaveFileWriter::WaveFileWriter(const wchar_t* strFileName)
: m_hFile(0)
, m_bHeaderWritten(false)
{
	open(strFileName);
}
#endif

WaveFile::WaveFileWriter::WaveFileWriter(const char* strFileName)
: m_hFile(0)
, m_bHeaderWritten(false)
{
	open(strFileName);
}

WaveFileChunks::WaveFileHeader& WaveFile::WaveFileWriter::getWaveFileHeader()
{
	return m_cHeader;
}

void WaveFile::WaveFileWriter::setWaveFileHeader(const WaveFileChunks::WaveFileHeader& header)
{
	m_cHeader = header;
}

#ifdef WIN32
bool WaveFile::WaveFileWriter::open(const wchar_t* strFileName)
{
	if(isOpened())
	{
		return false;
	}

	m_bHeaderWritten = false;

	m_hFile = _wfopen(strFileName, L"wb");

	return m_hFile != 0;
}
#endif

bool WaveFile::WaveFileWriter::open(const char* strFileName)
{
	if(isOpened())
	{
		return false;
	}

	m_bHeaderWritten = false;

	m_hFile = fopen(strFileName, "wb");

	if (m_hFile == 0)
        return false;

	if (sizeof(m_cHeader) != fwrite(&m_cHeader, 1, sizeof(m_cHeader), m_hFile))
	{
	    fclose(m_hFile);
	    return false;
	}

	return true;
}

bool WaveFile::WaveFileWriter::isOpened() const
{
	return m_hFile != 0;
}

bool WaveFile::WaveFileWriter::close()
{
	if(isOpened())
	{
		int res = fclose(m_hFile);
		if(res == 0)
		{
			m_hFile = 0;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool WaveFile::WaveFileWriter::writeHeader()
{
	if(!isOpened())
	{
		return false;
	}
	if(m_bHeaderWritten)
	{
		return false;
	}

	m_bHeaderWritten = true;

    // goto the beginning of the file
	fseek(m_hFile, 0, SEEK_SET);

    // write header
	bool succ = sizeof(m_cHeader) == fwrite(&m_cHeader, 1, sizeof(m_cHeader), m_hFile);

    // seek to the end
	fseek(m_hFile, 0, SEEK_END);
	return succ;
}

WaveFile::WaveFileWriter::~WaveFileWriter()
{
	if(isOpened())
	{
		close();
	}
}

size_t WaveFile::WaveFileWriter::MusicFwrite(const void* SrcBuf, size_t ElementSize, size_t Count)
{
	return fwrite(SrcBuf, ElementSize, Count, m_hFile);
}

#undef _CRT_SECURE_NO_DEPRECATE
