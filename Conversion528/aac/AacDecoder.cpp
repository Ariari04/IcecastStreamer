#include <aac/AacDecoder.h>

#include <string.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <faad_imported/main_imported.h>

namespace Decoding
{
	AacDecoder::AacDecoder()
	{
	}

	AacDecoder::~AacDecoder()
	{
	}

	int AacDecoder::open(const char* fileName)
	{
		//std::string additionalOutputFile = std::string(fileName) + ".wav";
		int argc = 3;
		char* argv[3] = { "", "-f 2", (char*)fileName };

		int ret = faad_open_decoding(argc, argv);

		firstZero = true;

		return 1 - ret;
	}

	int AacDecoder::read(char* Buffer, size_t Count)
	{
		int iread = faad_iteration_decoding(Buffer, Count);

		if (firstZero && iread == 0)
		{
			iread = faad_iteration_decoding(Buffer, Count);
		}

		if (iread < 1)
		{
			faad_close_decoding();
			return -1;
		}

		firstZero = false;

		return iread;
	}
}
