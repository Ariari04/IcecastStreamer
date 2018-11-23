#define _USE_MATH_DEFINES
#include "ExperimentalConverter.h"
#include <cassert>
#include <cstring>

#include <stdint.h>


#include <unsupported/Eigen/FFT>

#include <opencv2/opencv.hpp>
using namespace cv;


using namespace Conversion;

void hamming(int windowLength, std::vector<float>& buffer) {

	for (int i = 0; i < windowLength; i++) {
		buffer[i] = 0.54 - (0.46 * cos(2 * M_PI * (i / ((windowLength - 1) * 1.0))));
	}

}

uint32 ExperimentalConverter::getInputDozeSize() const
{
	return 2048;
	//return 100;
}

uint32 ExperimentalConverter::getOutputDozeSize() const
{
	return 2048;
	//return 100;
}

std::vector<std::vector<float>> outputDataArr;

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

	
	std::vector<float> hammingWindow;
	hammingWindow.resize(inputCount);

	hamming(inputCount, hammingWindow);
	
	for (size_t i = 0; i < inputCount; i++)
	{
		//outputSamples[i] = inputSamples[i] * 0.25;
		//float val = (1.0 + sin(100*i/float(inputCount) * M_PI))*0.5;
		
		//outputSamples[i] = val * 120;

		outputSamples[i] = inputSamples[i] * hammingWindow[i];
	}
	

	//memcpy(outputSamples, inputSamples, inputCount);
	
	
	

	std::vector<float> timeData;
	timeData.resize(2048);

	Eigen::FFT<float> fft;

	std::vector<std::complex<float> > freqvec;


	for (size_t i = 0; i < inputCount; i++)
	{
		timeData[i] = outputSamples[i] / 255.0;
	}

	if (inputCount > 0)
	{

		fft.fwd(freqvec, timeData);

		std::vector<float> outputData;
		outputData.resize(freqvec.size());

		for (size_t i = 0; i < freqvec.size(); i++)
		{
			outputData[i] = log(sqrt(freqvec[i].real()*freqvec[i].real() + freqvec[i].imag()*freqvec[i].imag()));
		}

		outputDataArr.push_back(outputData);

	}

	


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
