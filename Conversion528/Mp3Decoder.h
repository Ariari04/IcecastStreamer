#pragma once

#include <cstdio>
#include <vector>
#include "DecoderAdapter.h"
#include "Progress.h"

#include <Conversion.h>
#include <lame.h>

typedef struct mpstr_tag MPSTR;

namespace Decoding
{

class Mp3DecoderProducer : public BufferedProducerBase
{
public:
	lame_t gf;
	bool endOfFile;

	Mp3DecoderProducer();
	~Mp3DecoderProducer();

	void open(const char* fileName) override;
	void open(const wchar_t* fileName) override;

	bool readSamples(std::vector<byte>& samples) override;

	Conversion::SoundFormatInfo getSoundFormatInfo() const override;
};

}
