#include "WaveFileAdapter.h"

WaveFile::WaveProducer::WaveProducer(WaveFile::WaveFileReader &reader)
: m_cReader(reader)
{
	m_sSoundFormatInfo.nBitsPerSample = reader.getWaveFileHeader().m_cFormatChunk.m_nBitsPerSample;
	m_sSoundFormatInfo.nNumberOfChannels = reader.getWaveFileHeader().m_cFormatChunk.m_nChannels;
	m_sSoundFormatInfo.nSampleRate = reader.getWaveFileHeader().m_cFormatChunk.m_nSamplesPerSec;
}

Conversion::SoundFormatInfo WaveFile::WaveProducer::getSoundFormatInfo() const
{
	return m_sSoundFormatInfo;
}

uint32	WaveFile::WaveProducer::getSound(byte* pBuffer, uint32 nBufferSize)
{
	return 0;// uint32(m_cReader.MusicFread(pBuffer, 1, nBufferSize));
}

WaveFile::WaveConsumer::WaveConsumer(WaveFile::WaveFileWriter& writer)
: m_cWriter(writer)
, m_soundSize(0)
{
}
WaveFile::WaveConsumer::~WaveConsumer()
{
	m_cWriter.getWaveFileHeader().setSoundSizeInBytes(m_soundSize);
	m_cWriter.writeHeader();
}
bool WaveFile::WaveConsumer::setSoundFormatInfo(const Conversion::SoundFormatInfo& sInfo)
{
	WaveFileChunks::WaveFileHeader header;
	if(!header.m_cFormatChunk.setBitsPerSample(sInfo.nBitsPerSample))
	{
		return false;
	}
	if(!header.m_cFormatChunk.setNumberOfChannels(sInfo.nNumberOfChannels))
	{
		return false;
	}
	if(!header.m_cFormatChunk.setSampleRate(sInfo.nSampleRate))
	{
		return false;
	}
	m_cWriter.setWaveFileHeader(header);
	return true;
}
uint32 WaveFile::WaveConsumer::putSound(const byte* pBuffer, uint32 nBufferSize)
{
    m_soundSize += nBufferSize;
	return 0;// (uint32)m_cWriter.MusicFwrite(pBuffer, 1, nBufferSize);
}

bool WaveFile::WaveConsumer::writeHeader()
{
	m_cWriter.getWaveFileHeader().setSoundSizeInBytes(m_soundSize);
	m_cWriter.writeHeader();
	
	return true;
}
