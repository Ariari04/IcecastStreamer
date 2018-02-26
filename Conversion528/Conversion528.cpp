// Conversion528.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "WaveFile.h"
#include "Conversion.h"
#include <memory.h>
#include "WaveFileAdapter.h"

#define _WRITE_ERROR_DESCR_
#ifdef _WRITE_ERROR_DESCR_

#include <fstream>
#include "ConsoleHelpers.h"

#endif

int main(int argc, char* argv[])
{
	WaveFile::WaveFileReader reader;
	if(!reader.open("input.wav"))
	{
		#ifdef _WRITE_ERROR_DESCR_
			std::ofstream ofs("result.txt");
			KString str;
			str.Format("Input was not opened (%d): ", reader.getLastError());
			str += WFREToString(reader.getLastError());
			ofs << (const char*) str;
			ofs.close();
		#endif

		return 1;
	}

	WaveFile::WaveFileWriter writer;

	if(!writer.open("output.wav"))
	{
		reader.close();
		return 1;
	}

	WaveFile::WaveProducer producer(reader);
	WaveFile::WaveConsumer consumer(writer);

	Conversion528::ConversionResult res =
		Conversion528::ConversionProdCons(consumer, producer);

	#ifdef _WRITE_ERROR_DESCR_
		std::ofstream ofs("result.txt");
		ofs << (const char*)ConversionResultToString(res);
		ofs.close();
	#endif

	reader.close();
	writer.close();


	return 0;
}

/*
int oldMain()
{
	WaveFile::WaveFileReader reader;
	bool b = false;
	b = reader.open(L"input.wav");
	if(!b)
	{
		return 1;
	}

	int nBpSam =reader.getWaveFileHeader().m_cFormatChunk.m_nBitsPerSample;

	size_t BytesOfMusic = reader.getWaveFileHeader().getSoundSizeInBytes();
	size_t Samples = BytesOfMusic / 4;

	size_t nDozes = Samples / 111;
	size_t remainingSamples = Samples%111;

	size_t OnePart = 1000; //Dozes.

	size_t num_iter = nDozes / OnePart;
	size_t remainDozes = nDozes % OnePart;

	Conversion528::InterleavedSample<Conversion528::Sample16> * pInput = new \
			Conversion528::InterleavedSample<Conversion528::Sample16>[OnePart * 111];


	Conversion528::InterleavedSample<Conversion528::Sample16> * pOutput = new \
			Conversion528::InterleavedSample<Conversion528::Sample16>[OnePart * 110];



	WaveFile::WaveFileWriter writer;

	if(!writer.open(L"output.wav"))
	{
		reader.close();
		delete pInput;
		delete pOutput;
		return 0;
	}

	writer.setWaveFileHeader(reader.getWaveFileHeader());
	writer.getWaveFileHeader().setSoundSizeInBytes(4*110*(nDozes+1));

	size_t bWr = 0;

	if(!writer.writeHeader())
	{
		reader.close();
		writer.close();
		delete pInput;
		delete pOutput;
		return 0;
	}

	for(unsigned int i = 0; i<num_iter; ++i)
	{
		size_t nToRead = OnePart*111*4;
		size_t nToWrite = OnePart*110*4;

		if(nToRead != reader.MusicFread(pInput, 1,nToRead))
		{
			reader.close();
			writer.close();
			delete pInput;
			delete pOutput;
			return 0;
		}

		b = Conversion528::Convert528(pInput, OnePart*111, pOutput, OnePart*110);
		if(!b)
		{
			reader.close();
			writer.close();
			delete pInput;
			delete pOutput;
			return 0;
		}

		bWr = writer.MusicFwrite(pOutput, 1, nToWrite);
		if(bWr != nToWrite)
		{
			reader.close();
			writer.close();
			delete pInput;
			delete pOutput;
			return 0;
		}

	}

	//remain dozes.
	{
		size_t nToRead = remainDozes*111*4;
		size_t nToWrite = remainDozes*110*4;

		if(nToRead != reader.MusicFread(pInput, 1,nToRead))
		{
			reader.close();
			writer.close();
			delete pInput;
			delete pOutput;
			return 0;
		}

		b = Conversion528::Convert528(pInput, remainDozes*111, pOutput, remainDozes*110);
		if(!b)
		{
			reader.close();
			writer.close();
			delete pInput;
			delete pOutput;
			return 0;
		}

		bWr = writer.MusicFwrite(pOutput, 1, nToWrite);
		if(bWr != nToWrite)
		{
			reader.close();
			writer.close();
			delete pInput;
			delete pOutput;
			return 0;
		}
	}

	//Remaining Samples.
	{
		memset(pInput,0,4*111);

		size_t nToRead = remainingSamples*4;
		size_t nToWrite = 110*4;

		if(nToRead != reader.MusicFread(pInput, 1,nToRead))
		{
			reader.close();
			writer.close();
			delete pInput;
			delete pOutput;
			return 0;
		}

		b = Conversion528::Convert528(pInput, 111, pOutput, 110);
		if(!b)
		{
			reader.close();
			writer.close();
			delete pInput;
			delete pOutput;
			return 0;
		}

		bWr = writer.MusicFwrite(pOutput, 1, nToWrite);
		if(bWr != nToWrite)
		{
			reader.close();
			writer.close();
			delete pInput;
			delete pOutput;
			return 0;
		}

	}

	reader.close();
	writer.close();

	return 0;
}

*/
