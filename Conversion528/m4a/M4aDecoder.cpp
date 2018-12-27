#include <m4a/M4aDecoder.h>

#include <string.h>
#include <cstdlib>

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

		faad_open_decoding(argc, argv);

		while (true)
		{
			int iread = decodeAacfile_iteration();

			if (iread < 1)
			{
				decodeAacfile_closing();
				break;
				return -1;
			}
		}

		return 0;
	}

	int M4aDecoder::read(char* Buffer, size_t Count)
	{
		return 0;
	}
}
