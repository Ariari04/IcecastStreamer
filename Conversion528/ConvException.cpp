#include "ConvException.h"
#include "StaticAssertion.h"

std::string ConvException::errorCodeToString(ConvException::ErrorCode err)
{
	StaticAssert(LastError == 11);


	if (!(NoError <= err && err < LastError))
	{
		return "unknown error";
	}

		/* the enum
		NoError = 0
		FileNotFound = 1
		FileHasInvalidFormat = 2
		FileFormatNotSupported = 3
		InvalidConversionResult = 4
		UnsupportedBitDepth = 5
		UnsupportedChannels = 6
		UnsupportedSamplerate = 7
		UnsupportedOutputFormat = 8
		CoreConversionError = 9
		EncodingError = 10
		*/

	const char* str_arr[LastError] =
	{
		"no error",
		"file not found",
		"file has invalid format",
		"file format is not supported",
		"invalid conversion result",
		"unsupported bits per sample",
		"not a stereo sound",
		"unsupported samplerate",
		"encoder does not support output format",
		"core conversion error",
		"encoder error",
	};

	return str_arr[err];
}
