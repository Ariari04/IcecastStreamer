/* Copyright Zaurmann Software 2010 */
#include "Conversion.h"
#include <vector>
#include <cstring>

using namespace Conversion;

ConversionResult Conversion::ConversionProdCons(ISoundConverter& converter,
									ISoundConsumer& consumer,
									ISoundProducer& producer)
{
	uint16 nBitsPerSample = producer.getSoundFormatInfo().nBitsPerSample;
	if(!(nBitsPerSample == 8 || nBitsPerSample == 16 || nBitsPerSample == 32  || nBitsPerSample == 64))
	{
		return CRUnsupportedBitsPerSampleInput;
	}

	uint16 nBytesPerSample = nBitsPerSample / 8;

	uint16 nNumberOfChannels = producer.getSoundFormatInfo().nNumberOfChannels;
	
	/*if(nNumberOfChannels != 2)
	{
		return CRNotAStereoSound;
	}*/

	uint32 nSampleRate = producer.getSoundFormatInfo().nSampleRate;

	/*
	if(!(nSampleRate == 44100 || nSampleRate == 48000 || nSampleRate == 96000))
	{
		return CRUnsupportedSampleRate;
	}*/

	// Now input format is checked

	if(! consumer.setSoundFormatInfo(producer.getSoundFormatInfo()))
	{
		return CRConsumerDoesNotSupportOutputFormat;
	}

	const uint32 nInputDozeSize = converter.getInputDozeSize();
	const uint32 nOutputDozeSize = converter.getOutputDozeSize();

	// Subject to changes after format additions.
	//const uint32 nDozesInAFragment = 1000;
	const uint32 nDozesInAFragment = 1;

	//Int means Interleaved
	const uint32 nIntSampleSizeInBytes = nBytesPerSample * nNumberOfChannels;

	// Able to write now.

	ConversionFuncPointerType MyConversionFunc = converter.getConversionFunc(nBitsPerSample);

	if (MyConversionFunc == 0)
		return CRUnsupportedBitsPerSampleInput;

	std::vector<byte> vInput(nDozesInAFragment * nInputDozeSize * nIntSampleSizeInBytes);
	std::vector<byte> vOutput(nDozesInAFragment * nOutputDozeSize * nIntSampleSizeInBytes);

	byte * pInput = &(vInput[0]);
	byte * pOutput = &(vOutput[0]);

	size_t nToRead, nToWrite;

	//Bytes for producer and consumer
	nToRead = vInput.size();

	bool finished = false;

	while(!finished)
	{
	    const uint32 nReaded = producer.getSound(pInput, nToRead);
	    const uint32 nReadedSamples = nReaded / nIntSampleSizeInBytes;
	    const uint32 nReadedDozes = (nReadedSamples + 1) / nInputDozeSize;

        if (nReaded < nToRead)
        {
            memset(pInput + nReaded, 0, nToRead - nReaded);
            finished = true;
			
        }


			// Size in number of samples here.
			if (!MyConversionFunc(pInput, nReadedDozes * nInputDozeSize,
				pOutput, nReadedDozes * nOutputDozeSize))
			{
				return CRCoreConversionError;
			}

			nToWrite = nReadedDozes * nOutputDozeSize * nIntSampleSizeInBytes;

			if (nToWrite != consumer.putSound(pOutput, nToWrite))
			{
				return CRConsumerError;
			}
		
	}

	consumer.writeHeader();

	return CROk;
}
