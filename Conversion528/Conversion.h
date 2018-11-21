#ifndef _MY_528HZ_CONVERSION_H_
#define _MY_528HZ_CONVERSION_H_

/* Copyright Zaurmann Software 2010 */
#include <cstdlib>
#include "StaticAssertion.h"
#include "IntegerTypes.h"

namespace Conversion
{
	typedef int8  Sample8;
	typedef int16 Sample16;
	typedef int32 Sample32;
	typedef int64 Sample64;

#pragma pack(push,1)

	template <typename SampleType>
	struct InterleavedSample
	{
		SampleType left;
		SampleType right;
	};

#pragma pack(pop)

#define CheckInterleavedSampleSize(MySampleType) \
	StaticAssert(sizeof(InterleavedSample<MySampleType>) == 2*sizeof(MySampleType))

	CheckInterleavedSampleSize(Sample8);
	CheckInterleavedSampleSize(Sample16);
	CheckInterleavedSampleSize(Sample32);
	CheckInterleavedSampleSize(Sample64);


	typedef size_t index_t;

	struct SoundFormatInfo
	{
		uint32 nSampleRate;
		uint16 nNumberOfChannels;
		uint16 nBitsPerSample; //of one channel, one measurement.
	};

	typedef
	bool (*ConversionFuncPointerType)(byte* inputSamples, size_t inputCount,
		byte* outputSamples, size_t outputCount);

	class ISoundConverter
	{
	public:
		virtual uint32 getInputDozeSize() const = 0;
		virtual uint32 getOutputDozeSize() const = 0;
		virtual ConversionFuncPointerType getConversionFunc(int nBitsPerSample) = 0;
		virtual ~ISoundConverter() {}
	};

	class ISoundProducer
	{
	public:
		virtual SoundFormatInfo getSoundFormatInfo() const = 0;
		virtual uint32	getSound(byte* pBuffer, uint32 nBufferSize) = 0;
	};

	// Must be used only in this order:
	// 1. setSoundFormatInfo
	// 2. setSoundSizeInBytes
	// 3. putSound .. putSound .. putSound ..
	class ISoundConsumer
	{
	public:
		virtual bool setSoundFormatInfo(const SoundFormatInfo& sInfo) = 0;
		virtual uint32 putSound(const byte* pBuffer, uint32 nBufferSize) = 0;
		virtual bool writeHeader() { return true; }
	};

	enum ConversionResult
	{
		CROk = 0,
		CRUnsupportedBitsPerSampleInput,
		CRNotAStereoSound,
		CRUnsupportedSampleRate,
		CRConsumerDoesNotSupportOutputFormat,
		CRCoreConversionError,
		CRConsumerError,
		//Correct also ConsoleHelpers on additions.
		CR_last,
	};

	ConversionResult ConversionProdCons(ISoundConverter& converter, ISoundConsumer& consumer, ISoundProducer& producer);
}

#endif
