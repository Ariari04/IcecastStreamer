#include "AacDecoder.h"
#include "3rd/faad/faadecoder.h"
#include <cstring>
#include <stdexcept>

#define SAMPLE_SIZE 4608
#define PIECE_SIZE 100

namespace Decoding
{

AacDecoderProducer::AacDecoderProducer(IProgressManager* progress)
    :m_hFile(0)
    ,m_fileInfo(new aacheaderInfo)
    ,m_ppData(0)
    ,m_progress(progress)
{
}

AacDecoderProducer::~AacDecoderProducer()
{
    if (m_hFile)
    {
        FAADUnInit(m_hFile);
    }

    delete m_fileInfo;
}

Conversion::SoundFormatInfo AacDecoderProducer::getSoundFormatInfo() const
{
	Conversion::SoundFormatInfo info;
    info.nSampleRate = m_fileInfo->samplingRate;
    info.nNumberOfChannels = m_fileInfo->channelMode;
    info.nBitsPerSample = m_fileInfo->bitspersample;
    return info;
}

bool AacDecoderProducer::readSamples(std::vector<byte>& samples)
{
    char* intBuffer = 0;
    int intBufferLength = 0;
    long nCurrentTime = 0;

    if (FAADGetNextSample(m_hFile, &intBuffer, &intBufferLength, &nCurrentTime) != -1)
    {
        samples.resize(intBufferLength);
        memcpy(&samples.front(), intBuffer, intBufferLength);
        m_progress->reportProgress(nCurrentTime*1000, m_fileInfo->duration);
        return true;
    }
    else
    {
        return false;
    }
}

void AacDecoderProducer::open(const char* fileName)
{
    m_hFile = FAADInit(fileName, m_fileInfo);

    if (!m_hFile)
    {
		throw std::runtime_error("failed to open file");
    }
}

}
