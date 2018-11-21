#define _USE_MATH_DEFINES
#include "ExperimentalConverter.h"
#include <cassert>
#include <cstring>

#include <stdint.h>

using namespace Conversion;

uint32 ExperimentalConverter::getInputDozeSize() const
{
	return 2205;
}

uint32 ExperimentalConverter::getOutputDozeSize() const
{
	return 2205;
}

template<int SampleSize>
	bool flatConverterFunc(byte* inputSamples, size_t inputCount, byte* outputSamples, size_t outputCount)
{
	assert(inputCount == outputCount);

	/*
	static size_t randomLevel = 0;

	if (randomLevel == 0)
	{
		randomLevel = 1;
	}
	else
	{
		randomLevel = 0;
	}*/

	
	
	for (size_t i = 0; i < inputCount; i++)
	{
		//outputSamples[i] = inputSamples[i] * 0.25;
		float val = (1.0 + sin(100*i/float(inputCount) * M_PI))*0.5;
		
		outputSamples[i] = val * 120;
	}

	//memset(outputSamples, rand() % 256, inputCount*SampleSize * 2);
	//memcpy(outputSamples, inputSamples, inputCount);
	return true;
}

ConversionFuncPointerType ExperimentalConverter::getConversionFunc(int nBitsPerSample)
{
	switch(nBitsPerSample)
	{
	case 8: return &flatConverterFunc<1>;
	case 16: return &flatConverterFunc<2>;
	case 24: return &flatConverterFunc<3>;
	case 32: return &flatConverterFunc<4>;
	case 64: return &flatConverterFunc<8>;
	default:
		return 0;
	}
}
