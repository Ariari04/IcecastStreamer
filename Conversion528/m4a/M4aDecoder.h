#pragma once

//#include <interface/interface.h>
#include <cstdio>
#include <AudioFile.h>

namespace Decoding
{

	class M4aDecoder : public AudioDecoder // public BufferedProducerBase
	{
	public:
		void* m_hFile = NULL;
		//aacheaderInfo  m_fileInfo;

		M4aDecoder();
		~M4aDecoder();

		int open(const char* fileName) override;

		int read(char* Buffer, size_t Count) override;
	};

}