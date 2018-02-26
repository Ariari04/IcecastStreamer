#pragma once

#include <cstdio>
#include <string>
#include "EncoderAdapter.h"
#include "3rd/blade/BladeMP3Enc.h"

namespace Encoding
{

class Mp3EncoderConsumer : public EncoderConsumerBase
{
public:
    Mp3EncoderConsumer();
    ~Mp3EncoderConsumer();

	virtual bool setSoundFormatInfo(const Conversion::SoundFormatInfo& sInfo);
	virtual uint32 putSound(const byte* pBuffer, uint32 nBufferSize);

	static bool isAcceptableFormat(const Conversion::SoundFormatInfo& info);

protected:
    virtual void init(const char* fileName, const EncoderSettings& settings);

private:
    void finishEncoding();

    std::string m_fileName;
    FILE*       m_hFile;
    HBE_STREAM  m_hbeStream;
    BE_CONFIG   m_beConfig;
};

}
