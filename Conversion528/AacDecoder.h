#pragma once

#include "DecoderAdapter.h"
#include "3rd/faad/faadecoder.h"
#include <vector>
#include "Progress.h"

namespace Decoding
{

class AacDecoderProducer : public BufferedProducerBase
{
public:
    AacDecoderProducer(IProgressManager* progress);
    ~AacDecoderProducer();

	virtual Conversion::SoundFormatInfo getSoundFormatInfo() const;

protected:
    virtual bool readSamples(std::vector<byte>& samples);
    virtual void open(const char* fileName);

private:
	void*           m_hFile;
	aacheaderInfo*  m_fileInfo;
	char**          m_ppData;
	std::vector<byte> m_remaining;
	IProgressManager* m_progress;
};

}
