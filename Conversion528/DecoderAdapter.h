#ifndef DECODERADAPTER_H
#define DECODERADAPTER_H

#include "Conversion.h"
#include <vector>
#include "Progress.h"

namespace Decoding
{

enum DecoderKind
{
    Mp3 = 0,
    Aac,
};

class DecoderProducerBase : public Conversion::ISoundProducer
{
public:
    virtual ~DecoderProducerBase() {}

    static DecoderProducerBase* create(const char* fileName, IProgressManager* progress);

	virtual void open(const char* fileName) = 0;
	virtual void open(const wchar_t* fileName) = 0;
};

/*
class DecoratorProducerBase : public Conversion528::ISoundProducer
{
public:
    DecoratorProducerBase(Conversion528::ISoundProducer& baseProducer);

    virtual SoundFormatInfo getSoundFormatInfo() const;
	virtual uint32	getSound(byte* pBuffer, uint32 nBufferSize);

protected:
    Conversion528::ISoundProducer* m_baseProducer;
};
*/

class BufferedProducerBase : public DecoderProducerBase
{
public:
    BufferedProducerBase();

    virtual uint32	getSound(byte* pBuffer, uint32 nBufferSize);

	virtual bool readSamples(std::vector<byte>& samples) = 0;

private:
    std::vector<byte> m_intBuffer;
};

}

#endif
