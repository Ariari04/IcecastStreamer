#pragma once

#include "Conversion.h"
#include <vector>

namespace Encoding
{

enum EncoderType
{
    Mp3 = 0,
};

struct EncoderSettings
{
    enum BitrateType
    {
        VariableBitrate,
        ConstantBitrate,
    };

    EncoderSettings();

    BitrateType bitrateType;
    uint32      minBitrate;
    uint32      maxBitrate;
    //bool        stereo;
};

class EncoderConsumerBase : public Conversion::ISoundConsumer
{
public:
    EncoderConsumerBase();
    virtual ~EncoderConsumerBase() {}

    static EncoderConsumerBase* create(const char* fileName, const EncoderSettings& settings);

protected:
    virtual void init(const char* fileName, const EncoderSettings& settings) = 0;
};

class CompositeEncoderConsumer : public EncoderConsumerBase
{
public:
    CompositeEncoderConsumer();
    ~CompositeEncoderConsumer();

    void addConsumer(EncoderConsumerBase* consumer);

	virtual bool setSoundFormatInfo(const Conversion::SoundFormatInfo& sInfo);
	virtual uint32 putSound(const byte* pBuffer, uint32 nBufferSize);

private:
    virtual void init(const char* fileName, const EncoderSettings& settings);
    std::vector<EncoderConsumerBase*> m_consumers;
};

class LimitedEncoderConsumerDecorator : public EncoderConsumerBase
{
public:
	LimitedEncoderConsumerDecorator(EncoderConsumerBase* baseConsumer, int timeLimit);
	~LimitedEncoderConsumerDecorator();

	virtual bool setSoundFormatInfo(const Conversion::SoundFormatInfo& sInfo);
	virtual uint32 putSound(const byte* pBuffer, uint32 nBufferSize);

private:
	virtual void init(const char* fileName, const EncoderSettings& settings);

	EncoderConsumerBase* m_baseConsumer;
	uint32				m_timeLimit;
	uint32 				m_bytesWritten;
	uint32				m_bytesInSecond;
};

}
