#include "Converter528.h"

using namespace Conversion;

//Unstriaght template function selection.

typedef
bool (*ConversionFuncPointerType8)(InterleavedSample<Sample8>* inputSamples, size_t inputCount,
	InterleavedSample<Sample8>* outputSamples, size_t outputCount);

typedef
bool (*ConversionFuncPointerType16)(InterleavedSample<Sample16>* inputSamples, size_t inputCount,
	InterleavedSample<Sample16>* outputSamples, size_t outputCount);

typedef
bool (*ConversionFuncPointerType32)(InterleavedSample<Sample32>* inputSamples, size_t inputCount,
	InterleavedSample<Sample32>* outputSamples, size_t outputCount);

typedef
bool (*ConversionFuncPointerType64)(InterleavedSample<Sample64>* inputSamples, size_t inputCount,
	InterleavedSample<Sample64>* outputSamples, size_t outputCount);

uint32 Converter528::getInputDozeSize() const
{
	return 111;
}

uint32 Converter528::getOutputDozeSize() const
{
	return 110;
}

ConversionFuncPointerType Converter528::getConversionFunc(int nBitsPerSample)
{
	switch(nBitsPerSample)
	{
	 case 8:
		 return (ConversionFuncPointerType)(ConversionFuncPointerType8)&Convert528<Sample8>;
	case 16:
		return (ConversionFuncPointerType)(ConversionFuncPointerType16)&Convert528<Sample16>;
	case 32:
		return (ConversionFuncPointerType)(ConversionFuncPointerType32)&Convert528<Sample32>;
	case 64:
		return (ConversionFuncPointerType)(ConversionFuncPointerType64)&Convert528<Sample64>;
	default:
		return 0;
	}
}
