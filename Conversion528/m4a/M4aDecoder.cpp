#include <m4a/M4aDecoder.h>

#include <string.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <faad_imported/main_imported.h>

namespace Decoding
{
	M4aDecoder::M4aDecoder()
	{
	}

	M4aDecoder::~M4aDecoder()
	{
	}

	int M4aDecoder::open(const char* fileName)
	{
		//std::string additionalOutputFile = std::string(fileName) + ".wav";
		int argc = 3;
		char* argv[3] = { "", "-f 2", (char*)fileName };

		int ret = faad_open_decoding(argc, argv);

		firstZero = true;

		return 1 - ret;
	}

	int M4aDecoder::read(char* Buffer, size_t Count)
	{
		int iread = decodeAacfile_iteration(Buffer, Count);

		if (firstZero && iread == 0)
		{
			iread = decodeAacfile_iteration(Buffer, Count);
		}

		if (iread < 1)
		{
			decodeAacfile_closing();
			return -1;
		}

		firstZero = false;

		return iread;
	}
}
