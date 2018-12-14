#pragma once

#include <cstdio>
#include <vector>
#include "DecoderAdapter.h"
#include "Progress.h"

typedef struct mpstr_tag MPSTR;

namespace Decoding
{

class Mp3DecoderProducer : public BufferedProducerBase
{
public:
    Mp3DecoderProducer(IProgressManager* progress);
	~Mp3DecoderProducer();

	virtual Conversion::SoundFormatInfo getSoundFormatInfo() const;

protected:
    virtual bool readSamples(std::vector<byte>& samples);
	void open(const char* fileName) override;
	void open(const wchar_t* fileName) override;

    void readFileInfo(const char* fileName);

private:
    bool    m_stereo;
    int     m_frequency;
    int 	m_start;

    size_t 	m_streamSize;
    size_t  m_bytesReaded;

    FILE*   m_file;
	MPSTR*  m_mp;
	std::vector<byte> m_remaining;
	IProgressManager* m_progress;
};

}
