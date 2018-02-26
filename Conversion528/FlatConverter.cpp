#include "FlatConverter.h"
#include <cassert>
#include <cstring>

using namespace Conversion;

uint32 FlatConverter::getInputDozeSize() const
{
	return 128;
}

uint32 FlatConverter::getOutputDozeSize() const
{
	return 128;
}

template<int SampleSize>
	bool flatConverterFunc(byte* inputSamples, size_t inputCount, byte* outputSamples, size_t outputCount)
{
	assert(inputCount == outputCount);
	memcpy(outputSamples, inputSamples, inputCount*SampleSize*2);
	return true;
}

ConversionFuncPointerType FlatConverter::getConversionFunc(int nBitsPerSample)
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
