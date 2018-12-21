#include "Mp3Decoder.h"

#include <fstream>

#include <lame_interface/imported.h>
#include <lame_interface/lame_interface.h>

namespace Decoding
{
	Mp3Decoder::Mp3Decoder()
	{
		gf = lame_init(); /* initialize libmp3lame */
	}

	Mp3Decoder::~Mp3Decoder()
	{
		if (oufFile)
		{
			fclose(oufFile);       /* close the output file */
			oufFile = NULL;
			lame_decoding_close();
		}

		lame_close(gf);
	}

	int Mp3Decoder::open(const char* fileName)
	{
		//lame_decoding_close();

		endOfFile = false;

		int argc = 5;
		char* argv[5] = { "", "--decode", "-t", "test/original.mp3", "test/raw.wav" };
		lame_main(gf, argc, argv, &oufFile);

		return 0;

		//return lame_decoding_open(gf, const_cast<char*>(fileName), &oufFile);
	}

	int Mp3Decoder::open(const wchar_t* fileName)
	{
		std::exception("Not implemented");

		return false;
	}

	int Mp3Decoder::read(char Buffer[2 * 1152 * 2], FILE* outFile)
	{
		//int iread = lame_decoding_read(gf, Buffer);
		int iread = lame_decoder_iter(gf, oufFile);

		if (!iread)
		{
			lame_decoding_close();
			fclose(oufFile);       /* close the output file */
			oufFile = NULL;
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
