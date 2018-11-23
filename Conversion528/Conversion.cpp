#define _USE_MATH_DEFINES

#include "Conversion.h"
#include <vector>
#include <cstring>


#include <unsupported/Eigen/FFT>

#include <opencv2/opencv.hpp>

using namespace Conversion;


std::vector<std::vector<float>> outputDataArr;

void hamming(int windowLength, std::vector<float>& buffer) {

	for (int i = 0; i < windowLength; i++) {
		buffer[i] = 0.54 - (0.46 * cos(2 * M_PI * (i / ((windowLength - 1) * 1.0))));
	}

}

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

	const uint32 nInputDozeSize = 128;
	const uint32 nOutputDozeSize = 128;

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




	std::vector<float> timeData;
	timeData.resize(nInputDozeSize);

	Eigen::FFT<float> fft;

	std::vector<float> hammingWindow;
	hammingWindow.resize(nInputDozeSize);

	std::vector<std::complex<float> > freqvec;

	hamming(nInputDozeSize, hammingWindow);


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
		
		for (size_t i = 0; i < nInputDozeSize; i++)
		{
			pOutput[i] = pInput[i] * hammingWindow[i];
			//pOutput[i] = pInput[i];
		}

		for (size_t i = 0; i < nInputDozeSize; i++)
		{
			timeData[i] = pOutput[i];
		}

		fft.fwd(freqvec, timeData);

		std::vector<float> outputData;
		outputData.resize(freqvec.size());

		for (size_t i = 0; i < freqvec.size(); i++)
		{
			outputData[i] = log(sqrt(freqvec[i].real()*freqvec[i].real() + freqvec[i].imag()*freqvec[i].imag()));
		}

		outputDataArr.push_back(outputData);



		nToWrite = nReadedDozes * nOutputDozeSize * nIntSampleSizeInBytes;

		if (nToWrite != consumer.putSound(pOutput, nToWrite))
		{
			return CRConsumerError;
		}
		
	}

	using namespace cv;

	
	cv::Mat m(nInputDozeSize, outputDataArr.size(), CV_8UC3);


	for (size_t k = 0; k < outputDataArr.size(); k++)
	{

		auto& outputData = outputDataArr[k];

		
			
		for (size_t i = 0; i < nInputDozeSize; i++)
		{
			//float val = (2.0 + outputData[i]) * 50;
			float val = (outputData[i]) * 50;

			val = min(255.0f, val);
			val = max(0.0f, val);

			Vec3b & color = m.at<Vec3b>(i, k);

			color(0) = val;

			color(1) = val;
			color(2) = val;
		}

	}

	imwrite("mat.png", m);

	consumer.writeHeader();

	return CROk;
}
