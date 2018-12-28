#include "wave/WaveEncoder.h"

#include <lame_imported/imported.h>
#include <lame_imported/parse_imported.h>

namespace Encoding
{
	WaveEncoder::~WaveEncoder()
	{
		close();
	}

	void WaveEncoder::close()
	{
		if (inFile) fclose(inFile);
		inFile = NULL;
		if (outFile) fclose(outFile);
		outFile = NULL;
	}

	int WaveEncoder::open(const char* inFileName, const char* outFileName)
	{
		close();

		inFile = fopen(inFileName, "rb");
		outFile = fopen(outFileName, "wb");

		if (!inFile || !outFile)
		{
			return 0;
		}

		if (sizeof(Header) != fwrite(&Header, 1, sizeof(Header), outFile))
		{
			close();
			return false;
		}

		return true;
	}

	int WaveEncoder::write(char* Buffer, size_t Count)
	{
		int readCount = fread(Buffer, 1, Count, inFile);
		int count = fwrite(Buffer, 1, readCount, outFile);

		if (count < 1)
		{
			close();
		}

		return count;
	}

}