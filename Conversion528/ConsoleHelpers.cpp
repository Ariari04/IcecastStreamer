#include "ConsoleHelpers.h"
#include "StaticAssertion.h"

KString ConversionResultToString(Conversion::ConversionResult cr)
{
	StaticAssert(Conversion::CR_last == 7);

	if(!(Conversion::CROk <= cr && cr < Conversion::CR_last))
	{
		return L"Unknown error";
	}

		/* the enum
		CROk = 0,
		CRUnsupportedBitsPerSampleInput = 1,
		CRNotAStereoSound = 2,
		CRUnsupportedSampleRate = 3,
		CRConsumerDoesNotSupportOutputFormat = 4,
		CRCoreConversionError = 5,
		CRConsumerError = 6,
		*/

	const wchar_t *str_arr[Conversion::CR_last] =
	{ L"Ok",
	  L"Unsupported bits per sample",
	  L"Not a stereo sound",
	  L"Unsupported samplerate",
	  L"Consumer does not support output format",
	  L"Core conversion error",
	  L"Consumer error",
	};

	return str_arr[cr];
}

KString WFREToString(WaveFile::WaveFileReaderError err)
{
	StaticAssert(WaveFile::WFRE_last == 6);
	if(!(WaveFile::WFREOk <= err && err < WaveFile::WFRE_last))
	{
		return L"Unknown error";
	}

		/* the enum
		WFREOk = 0,
		WFRECorruptedWaveFile,
		WFREAlreadyOpened,
		WFRECanNotOpenFile,
		WFREFileNotOpened,
		WFREUnsupportedFormat,
		//Correct console helpers before any addition here.
		WFRE_last
		*/

	const wchar_t *str_arr[WaveFile::WFRE_last] =
	{ L"Ok",
	  L"Corrupted or unsupported wave file",
	  L"Already opened",
	  L"Can not open file",
	  L"File not opened",
	  L"Unsupported format",
	};

	return str_arr[err];
}
