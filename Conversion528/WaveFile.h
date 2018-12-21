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

namespace WaveFile
{
	enum WaveFileReaderError
	{
		WFREOk = 0,
		WFRECorruptedWaveFile,
		WFREAlreadyOpened,
		WFRECanNotOpenFile,
		WFREFileNotOpened,
		WFREUnsupportedFormat,
		//Correct console helpers before any addition here.
		WFRE_last
	};

class WaveFileReader : public AudioFileReader
{
public:
	WaveFileReader();
#ifdef WIN32
	WaveFileReader(const wchar_t* strFileName);
#endif
	WaveFileReader(const char* strFileName);
	const WaveFileChunks::WaveFileHeader& getWaveFileHeader() const;

	int open(const wchar_t* strFileName) override;
	int open(const char* strFileName) override;

	bool close();
	bool isOpened() const;

	WaveFileReaderError getLastError() const;

	virtual ~WaveFileReader();

	int read(char Buffer[1152 * 2 * 2], FILE* outFile);
	int    isEof();

private:
    bool openImpl();

	FILE * m_hFile;
	WaveFileChunks::WaveFileHeader m_cHeader;
	WaveFileReaderError m_nLastError;
};

class WaveFileWriter : public AudioFileWriter
{
public:
	WaveFileWriter();
#ifdef WIN32
	WaveFileWriter(const wchar_t* strFileName);
#endif
	WaveFileWriter(const char* strFileName);
	WaveFileChunks::WaveFileHeader& getWaveFileHeader();
	void setWaveFileHeader(const WaveFileChunks::WaveFileHeader& header);
#ifdef WIN32
	int open(const wchar_t* strFileName);
#endif
	int open(const char* strFileName);
	bool close();
	bool isOpened() const;
	bool writeHeader();
	int write(int Buffer[2][1152], size_t ElementSize, size_t Count, FILE* outFile);
	virtual ~WaveFileWriter();

private:
    bool openImpl();
	FILE * m_hFile;
	WaveFileChunks::WaveFileHeader m_cHeader;
	bool m_bHeaderWritten;
};

}




#endif
