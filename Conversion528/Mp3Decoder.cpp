#include "Mp3Decoder.h"

#include <lame.h>
#include <lame_interface/parse_imported.h>
#include <lame_interface/lame_interface.h>

namespace Decoding
{
	Mp3DecoderProducer::Mp3DecoderProducer()
	{
		gf = lame_init(); /* initialize libmp3lame */
	}

	Mp3DecoderProducer::~Mp3DecoderProducer()
	{
		lame_decoding_close();
		lame_close(gf);
	}

	void Mp3DecoderProducer::open(const char* fileName)
	{
		lame_decoding_close();

		endOfFile = false;

		const int MAX_NOGAP = 1;
		const int PATH_MAX = 1023;
		int argc = 3;
		const char* argv[3]{ "", "-V2", fileName };
		char inPath[PATH_MAX + 1];
		char outPath[PATH_MAX + 1];
		//char *nogap_inPath[MAX_NOGAP];
		int max_nogap = 0;

		int ret = parse_args(gf, argc, const_cast<char**>(argv), inPath, outPath, nullptr, &max_nogap);

		lame_decoding_open(gf, const_cast<char*>(fileName));
	}

	void Mp3DecoderProducer::open(const wchar_t* fileName)
	{
		std::exception("Not implemented");
	}

	bool Mp3DecoderProducer::readSamples(std::vector<byte>& samples)
	{
		int Buffer[2][1152];
		int iread = lame_decoding_read(gf, Buffer);

		if (!iread)
		{
			lame_decoding_close();
			endOfFile = true;
			return false;
		}

		return true;
	}

	Conversion::SoundFormatInfo Mp3DecoderProducer::getSoundFormatInfo() const
	{
		Conversion::SoundFormatInfo info;
		//info.nSampleRate = m_frequency;
		//info.nNumberOfChannels = m_stereo ? 2 : 1;
		//info.nBitsPerSample = 16;
		return info;
	}
}
