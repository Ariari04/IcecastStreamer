#include <m4a/M4aDecoder.h>

#include <string.h>
#include <cstdlib>

namespace Decoding
{
	M4aDecoder::M4aDecoder()
	{
	}

	M4aDecoder::~M4aDecoder()
	{
		if (m_hFile)
		{
			FAADUnInit(m_hFile);
			m_hFile = NULL;
		}
	}

	int M4aDecoder::open(const char* fileName)
	{
#ifndef WIN32
		FAADCHAR* faadFileName = (FAADCHAR*)fileName
#else
		const size_t cSize = strlen(fileName) + 1;
		FAADCHAR* faadFileName = new FAADCHAR[cSize];
		std::mbstowcs(faadFileName, fileName, cSize);
#endif

		m_hFile = FAADInit(faadFileName, &m_fileInfo);
		return m_hFile != NULL;
	}

	int M4aDecoder::read(char* Buffer, size_t Count)
	{
		int intBufferLength = 0;
		long nCurrentTime = 0;

		char* dynamicBuffer;

		if (FAADGetNextSample(m_hFile, &dynamicBuffer, &intBufferLength, &nCurrentTime) != -1)
		{
			memcpy(Buffer, dynamicBuffer, intBufferLength);
			//delete dynamicBuffer;
			return intBufferLength;
		}
		else
		{
			return -1;
		}
	}
}
