#include "Mp3Decoder.h"

#include <fstream>

#include <lame_imported/imported.h>

namespace Decoding
{
	Mp3Decoder::Mp3Decoder()
	{
		oufFile = NULL;
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
		wavsize = 0;

		int argc = 5;
		char* argv[5] = { "", "--decode", "-t", "test/original.mp3", "test/raw.wav" };
		if (lame_main_imported(gf, argc, argv, &oufFile))
		{
			return 0;
		}

		if (open_decoding(gf, &oufFile))
		{
			return 0;
		}

		return 1;
	}

	int Mp3Decoder::read(char* Buffer, size_t Count)
	{
		int iread = lame_decoder_iter(gf, oufFile, Buffer, Count, &wavsize);

		if (!iread)
		{
			lame_decoding_close();
			fclose(oufFile);       /* close the output file */
			oufFile = NULL;
			return -1;
		}

		return iread;
	}
}
