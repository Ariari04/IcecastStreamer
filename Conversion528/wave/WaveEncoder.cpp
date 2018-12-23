#include "wave/WaveEncoder.h"

#include <lame_imported/imported.h>
#include <lame_imported/parse_imported.h>

namespace Encoding
{
	WaveEncoder::~WaveEncoder()
	{
		if (file)
		{
			close();
		}
	}

	void WaveEncoder::close()
	{
		fclose(file);
		file = NULL;
	}

	int WaveEncoder::open(const char* fileName)
	{
		if (file)
		{
			close();
		}

		file = fopen(fileName, "wb");

		if (!file)
		{
			return 0;
		}

		if (sizeof(Header) != fwrite(&Header, 1, sizeof(Header), file))
		{
			close();
			return false;
		}

		return true;
	}

	int WaveEncoder::write(char* Buffer, size_t Count)
	{
		int count = fwrite(Buffer, 1, Count, file);

		if (count < 1)
		{
			close();
		}

		return count;
	}

}