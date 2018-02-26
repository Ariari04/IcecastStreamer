#include "EncoderAdapter.h"
#include "Mp3Encoder.h"
#include "ConvException.h"
#include "PathOperations.h"
#include <memory>
#include <cassert>
#include <cstring>

namespace Encoding
{

EncoderSettings::EncoderSettings()
    :bitrateType(ConstantBitrate)
    ,minBitrate(64)
    ,maxBitrate(64)
//    ,stereo(true)
{
}

EncoderConsumerBase::EncoderConsumerBase()
{
}

EncoderConsumerBase* EncoderConsumerBase::create(const char* fileName, const EncoderSettings& settings)
{
    std::auto_ptr<EncoderConsumerBase> consumer;

    std::string strExt = PathOperations::GetExtension(fileName);

    if (strcasecmp(strExt.c_str(), "mp3") == 0)
    {
		consumer.reset(new Mp3EncoderConsumer);
    }
	else
	{
		throw ConvException(ConvException::FileFormatNotSupported, fileName);
	}

	consumer->init(fileName, settings);
	return consumer.release();
}

CompositeEncoderConsumer::CompositeEncoderConsumer()
{
}

CompositeEncoderConsumer::~CompositeEncoderConsumer()
{
    for(size_t i = 0; i < m_consumers.size(); ++i)
        delete m_consumers.at(i);

    m_consumers.clear();
}

void CompositeEncoderConsumer::addConsumer(EncoderConsumerBase* consumer)
{
    assert(consumer);

    if (consumer)
    {
        m_consumers.push_back(consumer);
    }
}

bool CompositeEncoderConsumer::setSoundFormatInfo(const Conversion::SoundFormatInfo& sInfo)
{
    bool ret = true;

    for(size_t i = 0; i < m_consumers.size(); ++i)
    {
        ret &= m_consumers.at(i)->setSoundFormatInfo(sInfo);
    }
    return ret;
}

uint32 CompositeEncoderConsumer::putSound(const byte* pBuffer, uint32 nBufferSize)
{
    bool success = true;

    for(size_t i = 0; i < m_consumers.size(); ++i)
    {
        uint32 nWritten = m_consumers.at(i)->putSound(pBuffer, nBufferSize);
        success &= (nBufferSize == nWritten);
    }

    return success ? nBufferSize : 0;
}

void CompositeEncoderConsumer::init(const char* /*fileName*/, const EncoderSettings& /*settings*/)
{
	// does nothing
}

LimitedEncoderConsumerDecorator::LimitedEncoderConsumerDecorator(EncoderConsumerBase* baseConsumer, int timeLimit)
	:m_baseConsumer(baseConsumer)
	,m_timeLimit(timeLimit)
	,m_bytesWritten(0)
	,m_bytesInSecond(0)
{
}

LimitedEncoderConsumerDecorator::~LimitedEncoderConsumerDecorator()
{
	delete m_baseConsumer;
}

bool LimitedEncoderConsumerDecorator::setSoundFormatInfo(const Conversion::SoundFormatInfo& sInfo)
{
	m_bytesInSecond = (sInfo.nSampleRate * sInfo.nNumberOfChannels * sInfo.nBitsPerSample) / 8;

	return m_baseConsumer->setSoundFormatInfo(sInfo);
}

uint32 LimitedEncoderConsumerDecorator::putSound(const byte* pBuffer, uint32 nBufferSize)
{
	if (m_bytesWritten < m_timeLimit*m_bytesInSecond)
	{
		uint32 nToWrite = std::min(m_timeLimit*m_bytesInSecond - m_bytesWritten, nBufferSize);
		uint32 nWritten = m_baseConsumer->putSound(pBuffer, nToWrite);
		m_bytesWritten += nWritten;

		if (nWritten == nToWrite)
			return nBufferSize;
		else
			return nWritten;
	}
	else
	{
		return nBufferSize;
	}
}

void LimitedEncoderConsumerDecorator::init(const char*, const EncoderSettings&)
{
}

}
