#define _USE_MATH_DEFINES

#include "Conversion.h"
#include <vector>
#include <cstring>


#include <unsupported/Eigen/FFT>

#include <opencv2/opencv.hpp>

using namespace Conversion;


void RealFFT(size_t windowLength, std::vector<double>& in, double* outReal, double* outImaginary)
{
	Eigen::FFT<double> fft;

	std::vector<std::complex<double> > freqvec;

	fft.fwd(freqvec, in);

	for (size_t i = 0; i < freqvec.size(); i++)
	{
		outReal[i] = freqvec[i].real();
		outImaginary[i] = freqvec[i].imag();
	}

	std::vector<double> rvec;
	rvec.resize(freqvec.size());

	for (size_t i = 0; i < freqvec.size(); i++)
	{
		rvec[i] = sqrt(freqvec[i].real()*freqvec[i].real() + freqvec[i].imag()*freqvec[i].imag());
	}


	std::vector<double> rvecx;

	
}


/*
 * PowerSpectrum
 *
 * This function uses RealFFTf() from RealFFTf.h to perform the real
 * FFT computation, and then squares the real and imaginary part of
 * each coefficient, extracting the power and throwing away the phase.
 *
 * For speed, it does not call RealFFT, but duplicates some
 * of its code.
 */

void PowerSpectrum(size_t windowLength, std::vector<double>& in, double* out)
{
	Eigen::FFT<double> fft;

	std::vector<std::complex<double> > freqvec;

	fft.fwd(freqvec, in);


	std::vector<double> rvec;
	rvec.resize(freqvec.size());

	for (size_t i = 0; i < freqvec.size(); i++)
	{
		out[i] = sqrt(freqvec[i].imag()*freqvec[i].imag() + freqvec[i].real()*freqvec[i].real());
		rvec[i] = out[i];
	}

	std::vector<double> rvecx;


}





void hamming(size_t windowLength, std::vector<double>& buffer) {

	std::vector<float> bufx;
	bufx.resize(windowLength);
	for (size_t i = 0; i < windowLength; i++) {
		bufx[i] = (0.54 - (0.46 * cos(2 * M_PI * (i / ((windowLength - 1) * 1.0)))));
		buffer[i] = buffer[i] * bufx[i];
	}
}

void hanning(size_t windowLength, std::vector<double>& buffer) {

	std::vector<float> bufx;
	bufx.resize(windowLength);
	for (size_t i = 0; i < windowLength; i++) {
		bufx[i] = 0.5 * (1 - cos(2 * M_PI*i / (windowLength-1)));
		buffer[i] = buffer[i] * bufx[i];
	}
}


/*
  This function computes the power (mean square amplitude) as
  a function of frequency, for some block of audio data.

  width: the number of samples
  calculates windowSize/2 frequency samples
*/

bool ComputeSpectrum(const double * data, size_t width, size_t windowSize,
	double rate, double *output, bool autocorrelation,
	int windowFunc)
{
	if (width < windowSize)
		return false;

	if (!data || !output)
		return true;

	std::vector<double> processed;

	processed.resize(windowSize);

	for (size_t i = 0; i < windowSize; i++)
	{
		processed[i] = double(0.0);
	}

	auto half = windowSize / 2;



	std::vector<double> in;

	in.resize(windowSize);

	std::vector<double> out;

	out.resize(windowSize);


	std::vector<double> out2;

	out2.resize(windowSize);


	size_t start = 0;
	unsigned windows = 0;
	while (start + windowSize <= width) {
		for (size_t i = 0; i < windowSize; i++)
			in[i] = data[start + i];

		//hamming(windowSize, in);
		//hanning(windowSize, in);

		if (autocorrelation) {
			// Take FFT
			RealFFT(windowSize, in, &out[0], &out2[0]);
			// Compute power
			for (size_t i = 0; i < windowSize; i++)
			{
				in[i] = (out[i] * out[i]) + (out2[i] * out2[i]);
			}

			// Tolonen and Karjalainen recommend taking the cube root
			// of the power, instead of the square root

			for (size_t i = 0; i < windowSize; i++)
				in[i] = powf(in[i], 1.0f / 3.0f);

			// Take FFT
			RealFFT(windowSize, in, &out[0], &out2[0]);
		}
		else
		{
			PowerSpectrum(windowSize, in, &out[0]);
		}

		// Take real part of result
		for (size_t i = 0; i < half; i++)
			processed[i] += out[i];

		start += half;
		windows++;
	}

	if (autocorrelation) {

		// Peak Pruning as described by Tolonen and Karjalainen, 2000
		/*
		 Combine most of the calculations in a single for loop.
		 It should be safe, as indexes refer only to current and previous elements,
		 that have already been clipped, etc...
		*/
		for (size_t i = 0; i < half; i++) {
			// Clip at zero, copy to temp array
			if (processed[i] < 0.0)
				processed[i] = float(0.0);
			out[i] = processed[i];
			// Subtract a time-doubled signal (linearly interp.) from the original
			// (clipped) signal
			if ((i % 2) == 0)
				processed[i] -= out[i / 2];
			else
				processed[i] -= ((out[i / 2] + out[i / 2 + 1]) / 2);

			// Clip at zero again
			if (processed[i] < 0.0)
				processed[i] = float(0.0);
		}

		// Reverse and scale
		for (size_t i = 0; i < half; i++)
			in[i] = processed[i] / (windowSize / 4);
		for (size_t i = 0; i < half; i++)
			processed[half - 1 - i] = in[i];
	}
	else {
		// Convert to decibels
		// But do it safely; -Inf is nobody's friend
		for (size_t i = 0; i < half; i++) {
			float temp = (processed[i] / windowSize / windows);
			if (temp > 0.0)
				processed[i] = 10 * log10(temp);
			else
				processed[i] = 0;
		}
	}

	for (size_t i = 0; i < half; i++)
		output[i] = processed[i];

	return true;
}



ConversionResult Conversion::ConversionOnlySpectral(ISoundProducer& producer, const std::string& outputFileName)
{
	uint16 nBitsPerSample = producer.getSoundFormatInfo().nBitsPerSample;
	if (!(nBitsPerSample == 8 || nBitsPerSample == 16 || nBitsPerSample == 32 || nBitsPerSample == 64))
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


	const uint32 nInputDozeSize = 256;
	const uint32 nOutputDozeSize = 256;

	// Subject to changes after format additions.
	//const uint32 nDozesInAFragment = 1000;
	const uint32 nDozesInAFragment = 1;

	//Int means Interleaved
	const uint32 nIntSampleSizeInBytes = nBytesPerSample * nNumberOfChannels;

	// Able to write now.

	std::vector<byte> vInput(nDozesInAFragment * nInputDozeSize * nIntSampleSizeInBytes);
	std::vector<byte> vOutput(nDozesInAFragment * nOutputDozeSize * nIntSampleSizeInBytes);

	byte * pInput = &(vInput[0]);
	byte * pOutput = &(vOutput[0]);

	size_t nToRead, nToWrite;

	//Bytes for producer and consumer
	nToRead = vInput.size();

	bool finished = false;




	std::vector<double> timeData;
	//timeData.resize(nInputDozeSize);
	

	while (!finished)
	{
		//ind++;
		const uint32 nReaded = producer.getSound(pInput, nToRead);
		const uint32 nReadedSamples = nReaded;
		const uint32 nReadedDozes = (nReadedSamples + 1) / nInputDozeSize;

		if (nReaded < nToRead)
		{
			memset(pInput + nReaded, 0, nToRead - nReaded);
			finished = true;

		}

		for (size_t i = 0; i < nInputDozeSize/2; i++)
		{
			uint16_t x = uint16_t(pInput[2 * i+1]) * 256 + pInput[2 * i];
			//timeData.push_back(int(int16_t(x)) / 32768.0);
			timeData.push_back(int(int16_t(x)));

			
		}


		nToWrite = nReadedDozes * nOutputDozeSize * nIntSampleSizeInBytes;
	}

	using namespace cv;



	
	size_t windowWidth = 256;

	std::vector<double> outputData;
	outputData.resize(windowWidth);

	size_t stepSize = 256;
	size_t partWidth = windowWidth;

	size_t count = 1;

	//size_t count = timeData.size() / stepSize - partWidth/ stepSize;

	/*for (size_t i = 0; i < 256; i++)
	{
		std::cout << "(" << timeData[i] << ", 0)" << std::endl;
	}*/

	timeData.resize(256);

	cv::Mat m(windowWidth/2, count, CV_8UC3);

	size_t x = 0;

	for (size_t k = 0; k < count; k++)
	{
		ComputeSpectrum(&timeData[k*stepSize], partWidth, windowWidth, 0.0, &outputData[0], true, 0);

		for (size_t i = 0; i < windowWidth/2; i++)
		{

			//float val = (outputData[i])*100;
			float val = (outputData[i]);
			//float val = (2 + log10(outputData[i]))*50;

			val = min(255.0f, val);
			val = max(0.0f, val);

			Vec3b & color = m.at<Vec3b>((windowWidth / 2)-1 - i, x);

			color(0) = val;

			color(1) = val;
			color(2) = val;
		}

		x++;

	}

	imwrite(outputFileName, m);

	return CROk;
}


ConversionResult Conversion::ConversionProdCons(ISoundConverter& converter,
									ISoundConsumer& consumer,
									ISoundProducer& producer)
{

	/*
	uint16 nBitsPerSample = producer.getSoundFormatInfo().nBitsPerSample;
	if(!(nBitsPerSample == 8 || nBitsPerSample == 16 || nBitsPerSample == 32  || nBitsPerSample == 64))
	{
		return CRUnsupportedBitsPerSampleInput;
	}

	uint16 nBytesPerSample = nBitsPerSample / 8;

	uint16 nNumberOfChannels = producer.getSoundFormatInfo().nNumberOfChannels;
	


	uint32 nSampleRate = producer.getSoundFormatInfo().nSampleRate;



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

	const size_t hammingWindowSize = 2048;

	std::vector<float> hammingWindow;
	hammingWindow.resize(hammingWindowSize);

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
			//pOutput[i] = pInput[i] * hammingWindow[i];
			pOutput[i] = pInput[i];
		}

		for (size_t i = 0; i < nInputDozeSize; i++)
		{
			//timeData.push_back(pInput[i]);
			timeData.push_back(pInput[i]/256.0);
		}
		

		nToWrite = nReadedDozes * nOutputDozeSize * nIntSampleSizeInBytes;

		if (nToWrite != consumer.putSound(pOutput, nToWrite))
		{
			return CRConsumerError;
		}
		
	}

	using namespace cv;

	size_t stepSize = 2048;

	cv::Mat m(hammingWindowSize / 2, (timeData.size() - hammingWindowSize)/ stepSize + 1, CV_8UC3);

	size_t x = 0;

	for (size_t k = 0; k < timeData.size() - hammingWindowSize; k+= stepSize)
	{
		std::vector<float> td(timeData.begin() + k, timeData.begin() + k + hammingWindowSize);

		for (size_t i = 0; i < td.size(); i++)
		{
			td[i] = td[i] * hammingWindow[i];
		}

		fft.fwd(freqvec, td);


		std::vector<float> outputData;
		outputData.resize(freqvec.size());

		for (size_t i = 0; i < freqvec.size(); i++)
		{
			//outputData[i] = log(sqrt(freqvec[i].real()*freqvec[i].real() + freqvec[i].imag()*freqvec[i].imag()));
			outputData[i] = (sqrt(freqvec[i].real()*freqvec[i].real() + freqvec[i].imag()*freqvec[i].imag()));
		}
		
		for (size_t i = 0; i < hammingWindowSize / 2; i++)
		{
			//float val = (2.0 + outputData[i]) * 50;
			//float val = (outputData[i]);// * 50;
			float val = (outputData[i + hammingWindowSize / 2]) * 50;

			val = min(255.0f, val);
			val = max(0.0f, val);

			Vec3b & color = m.at<Vec3b>(i, x);

			color(0) = val;

			color(1) = val;
			color(2) = val;
		}

		x++;

	}

	imwrite("mat.png", m);

	consumer.writeHeader();
	*/
	return CROk;
}
