#pragma once

#include "Conversion.h"

namespace Conversion
{
	template<typename SampleType>
	void CombineLinear(const InterleavedSample<SampleType>& s1,
		const InterleavedSample<SampleType>& s2, const double alpha,
		InterleavedSample<SampleType>& sOut)
	{
		const double beta = 1.0 - alpha;
		sOut.left = SampleType(beta*s1.left + alpha*s2.left);
		sOut.right = SampleType(beta*s1.right + alpha*s2.right);
	}



	// 440 hz convert to 444 hz.
	// 110 to 111.
	// So output samples are 110.
	template<typename SampleType>
	void Convert528_111(InterleavedSample<SampleType>* inputSamples,
		InterleavedSample<SampleType>* outputSamples)
	{
		const double Eps = 111.0 / 110.0;
		const double Delta = Eps - 1;
		double Alpha = Delta;

		outputSamples[0] = inputSamples[0];
		outputSamples[109] = inputSamples[110];

		++inputSamples;
		++outputSamples;

		for(index_t i = 1; i < 109; ++i)
		{
			CombineLinear(*inputSamples, *(inputSamples+1), Alpha, *outputSamples);
			Alpha += Delta;
			++inputSamples;
			++outputSamples;
		}
	}


	template<typename SampleType>
	bool Convert528(InterleavedSample<SampleType>* inputSamples, size_t inputCount,
		InterleavedSample<SampleType>* outputSamples, size_t outputCount)
	{
		//Without tail.
		if((inputCount%111) || (outputCount%110))
		{
			return false;
		}
		//The same number of parts;
		if((inputCount/111) != (outputCount/110))
		{
			return false;
		}

		size_t nParts = inputCount / 111;

		for (size_t i = 0; i < nParts; ++i)
		{
			Convert528_111(inputSamples, outputSamples);
			inputSamples += 111;
			outputSamples += 110;
		}

		return true;
	}

	class Converter528 : public ISoundConverter
	{
	public:
		virtual uint32 getInputDozeSize() const;
		virtual uint32 getOutputDozeSize() const;
		virtual ConversionFuncPointerType getConversionFunc(int nBitsPerSample);
	};

}
