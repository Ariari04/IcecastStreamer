#pragma once

class AudioFileReader
{
public:
	virtual ~AudioFileReader() = default;

	virtual bool open(const wchar_t* strFileName) = 0;
	virtual bool open(const char* strFileName) = 0;

	virtual size_t MusicFread(void* DstBuf, size_t ElementSize, size_t Count) = 0;
	virtual int MusicFeof() = 0;
};

class AudioFileWriter
{
public:
	virtual ~AudioFileWriter() = default;
	virtual bool writeHeader() = 0;
	virtual size_t MusicFwrite(const void* SrcBuf, size_t ElementSize, size_t Count) = 0;
};