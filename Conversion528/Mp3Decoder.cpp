#include "Mp3Decoder.h"

#include <lame_interface/parse_imported.h>
#include <lame_interface/lame_interface.h>

namespace Decoding
{
	Mp3Decoder::Mp3Decoder()
	{
		gf = lame_init(); /* initialize libmp3lame */
	}

	Mp3Decoder::~Mp3Decoder()
	{
		lame_decoding_close();
		lame_close(gf);
	}

	bool Mp3Decoder::open(const char* fileName)
	{
		lame_decoding_close();

		endOfFile = false;

		const int MAX_NOGAP = 1;
		int argc = 4;
		const char* argv[4]{ "", "-V2", "", fileName };
		char inPath[PATH_MAX + 1];
		char outPath[PATH_MAX + 1];
		//char *nogap_inPath[MAX_NOGAP];
		int max_nogap = 0;

		if (parse_args(gf, argc, const_cast<char**>(argv), inPath, outPath, nullptr, &max_nogap))
		{
			return false;
		}

		lame_decoding_open(gf, const_cast<char*>(fileName));

		return true;
	}

	bool Mp3Decoder::open(const wchar_t* fileName)
	{
		std::exception("Not implemented");

		return false;
	}

	int Mp3Decoder::read(void* DstBuf, size_t ElementSize, size_t Count, FILE* outFile)
	{
		int Buffer[2][1152];
		int iread = lame_decoding_read(gf, Buffer);

		if (!iread)
		{
			lame_decoding_close();
			endOfFile = true;
			return -1;
		}

		return iread;
	}

	int Mp3Decoder::isEof()
	{
		return endOfFile;
	}

	//Conversion::SoundFormatInfo Mp3Decoder::getSoundFormatInfo() const
	//{
	//	Conversion::SoundFormatInfo info;
	//	//info.nSampleRate = m_frequency;
	//	//info.nNumberOfChannels = m_stereo ? 2 : 1;
	//	//info.nBitsPerSample = 16;
	//	return info;
	//}
}
