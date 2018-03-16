#include "DecoderAdapter.h"
#include <cstring>
#include <vector>
#include <memory>
#include "Mp3Decoder.h"
#include "AacDecoder.h"
#include "ConvException.h"
#include "PathOperations.h"

namespace Decoding
{


DecoderProducerBase* DecoderProducerBase::create(const char* fileName, IProgressManager* progress)
{
	std::string strExt = PathOperations::GetExtension(fileName);

    std::auto_ptr<DecoderProducerBase> decoder;

    if (strcasecmp(strExt.c_str(), "mp3") == 0)
        decoder.reset( new Mp3DecoderProducer(progress) );
    else if (strcasecmp(strExt.c_str(), "aac") == 0||
			 strcasecmp(strExt.c_str(), "m4a") == 0 ||
			 strcasecmp(strExt.c_str(), "mp4") == 0)
        decoder.reset( new AacDecoderProducer(progress) );
	else
		throw ConvException(ConvException::FileFormatNotSupported, fileName);

    decoder->open(fileName);
    return decoder.release();
}

BufferedProducerBase::BufferedProducerBase()
{
}

uint32 BufferedProducerBase::getSound(byte* pBuffer, uint32 nBufferSize)
{
    uint32 nTotalDone = 0;

    if (m_intBuffer.size())
    {
        size_t copySize = std::min(nBufferSize, static_cast<unsigned int>(m_intBuffer.size()));

        memcpy(pBuffer, &m_intBuffer.front(), copySize);
        pBuffer += copySize;
        nBufferSize -= copySize;
        nTotalDone += copySize;
        m_intBuffer.clear();
    }

    while(nBufferSize > 0)
    {
        if (readSamples(m_intBuffer))
        {
            if (m_intBuffer.size() < nBufferSize)
            {
                memcpy(pBuffer, &m_intBuffer.front(), m_intBuffer.size());
                nBufferSize -= m_intBuffer.size();
                pBuffer += m_intBuffer.size();
                nTotalDone += m_intBuffer.size();
                m_intBuffer.clear();
            }
            else
            {
                memcpy(pBuffer, &m_intBuffer.front(), nBufferSize);
                nTotalDone += nBufferSize;

                memmove(&m_intBuffer[0], &m_intBuffer[nBufferSize], m_intBuffer.size() - nBufferSize);
                m_intBuffer.resize(m_intBuffer.size() - nBufferSize);

                nBufferSize = 0;
            }
        }
        else
        {
            // error decoding sample
            return nTotalDone;
        }
    }

	return nTotalDone;
}

}
