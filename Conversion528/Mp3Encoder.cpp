#include "Mp3Encoder.h"

#include <exception>

#include <lame_interface/parse_imported.h>
#include <lame_interface/lame_interface.h>

namespace Encoding
{

	Mp3Encoder::Mp3Encoder()
	{
		writeHeader = true;
		gf = lame_init(); /* initialize libmp3lame */
	}

	Mp3Encoder::~Mp3Encoder()
	{
		lame_encoding_close();
		lame_close(gf);
	}

	bool Mp3Encoder::open(const char* fileName)
	{
		lame_encoding_close();

		const int MAX_NOGAP = 1;
		int argc = 3;
		const char* argv[3]{ "", "-V2", fileName };
		char inPath[PATH_MAX + 1];
		char outPath[PATH_MAX + 1];
		//char *nogap_inPath[MAX_NOGAP];
		int max_nogap = 0;

		if(parse_args(gf, argc, const_cast<char**>(argv), inPath, outPath, nullptr, &max_nogap))
		{
			return false;
		}

		lame_encoding_open(gf, const_cast<char*>(fileName));

		return true;
	}

	bool Mp3Encoder::open(const wchar_t* fileName)
	{
		std::exception("Not implemented");

		return false;
	}

	int Mp3Encoder::write(const void* SrcBuf, size_t ElementSize, size_t Count, FILE* outFile)
	{
		int byteCount = 0;
		int Buffer[2][1152];
		int wroteBytes = lame_encoding_write(gf, Buffer, byteCount, outFile, writeHeader);

		writeHeader = false;

		if (!wroteBytes)
		{
			lame_encoding_close();
			return -1;
		}

		return wroteBytes;
	}

	//bool Mp3Encoder::isAcceptableFormat(const Conversion::SoundFormatInfo& info)
	//{
	//    return  (info.nNumberOfChannels == 1 || info.nNumberOfChannels == 2) &&
	//            info.nBitsPerSample == 16;
	//}

	//void Mp3Encoder::init(const char* fileName, const EncoderSettings& settings)
	//{
	//	   m_fileName = fileName;

	//	   m_beConfig.dwConfig = BE_CONFIG_LAME;
	//	   m_beConfig.format.LHV1.dwStructVersion = 1;
	//	   m_beConfig.format.LHV1.dwStructSize = sizeof(BE_CONFIG);

	//	   /// USE DEFAULT PSYCHOACOUSTIC MODEL
	//	   m_beConfig.format.LHV1.dwPsyModel = 0;

	//	   /// NO EMPHASIS TURNED ON
	//	   m_beConfig.format.LHV1.dwEmphasis = 0;

	//	   /// SET ORIGINAL FLAG
	//	   m_beConfig.format.LHV1.bOriginal = TRUE;

	//	   /// Write INFO tag
	//	   m_beConfig.format.LHV1.bWriteVBRHeader = FALSE;

	//	   /// No Bit resorvoir
	//	   m_beConfig.format.LHV1.bNoRes = TRUE;

	//	   m_beConfig.format.LHV1.bPrivate = TRUE;
	//	   m_beConfig.format.LHV1.bCopyright = TRUE;

	//	   ///���� ����� ����������� - CBR
	//	   if (settings.bitrateType == EncoderSettings::ConstantBitrate)
	//	   {
	//	   	m_beConfig.format.LHV1.bEnableVBR = FALSE;

	//	   	/// QUALITY PRESET SETTING, CBR = Constant Bit Rate
	//	   	m_beConfig.format.LHV1.nPreset = LQP_CBR;

	//	   	/// MINIMUM BIT RATE
	//	   	m_beConfig.format.LHV1.dwBitrate = settings.minBitrate;
	//	   }
	//	   ///���� ����� ����������� - VBR
	//	   else
	//	   {
	//	   	m_beConfig.format.LHV1.bEnableVBR = TRUE;
	//	   	m_beConfig.format.LHV1.nVbrMethod = VBR_METHOD_NEW;
	//	   	m_beConfig.format.LHV1.bWriteVBRHeader = TRUE;

	//	   	/// QUALITY PRESET SETTING, VBR = Variable Bit Rate
	//	   	m_beConfig.format.LHV1.dwBitrate = settings.minBitrate;
	//	   	m_beConfig.format.LHV1.dwMaxBitrate = settings.maxBitrate;
	//	   }

	//	   /// MPEG VERSION (I or II)
	//	   m_beConfig.format.LHV1.dwMpegVersion = (settings.minBitrate > 32) ? MPEG1 : MPEG2;
	//}


	//bool Mp3Encoder::setSoundFormatInfo(const Conversion::SoundFormatInfo& sInfo)
	//{
	//	if (!isAcceptableFormat(sInfo))
	//	    return false;

	//	/// OUTPUT IN STREO OR MONO
	//	m_beConfig.format.LHV1.nMode = sInfo.nNumberOfChannels == 1
	//	        ? BE_MP3_MODE_MONO
	//	        : BE_MP3_MODE_JSTEREO;

	//	/// INPUT FREQUENCY
	//	m_beConfig.format.LHV1.dwSampleRate = sInfo.nSampleRate;

	//	/// OUTPUT FREQUENCY, IF == 0 THEN DON'T RESAMPLE
	//	m_beConfig.format.LHV1.dwReSampleRate = 0;

	//	DWORD	   dwMP3Buffer;
	//	DWORD	   dwPCMBuffer;
	//	///������������� ������ �����������
	//	if (beInitStream(&m_beConfig, &dwPCMBuffer, &dwMP3Buffer, &m_hbeStream) != BE_ERR_SUCCESSFUL)
	//	{
	//	    //L"Initialisation of stream for MP3 encoding was failed"
	//	    return false;
	//	}

	//	m_hFile = fopen(m_fileName.c_str(), "wb");

	//	if (m_hFile == NULL)
	//	{
	//	    beCloseStream(m_hbeStream);
	//	    return false;
	//	}

	//	return true;
	//}

	//#define PIECE_SIZE_IN_BYTES 204800

	//void Mp3Encoder::finishEncoding()
	//{
	//	byte aOutput[PIECE_SIZE_IN_BYTES];
	//	DWORD dwOutput = PIECE_SIZE_IN_BYTES;

	//	if (beDeinitStream(m_hbeStream, aOutput, &dwOutput) != BE_ERR_SUCCESSFUL)
	//	{
	//	    //L"Deinitialisation of stream for MP3 encoding was failed"
	//	    beCloseStream(m_hbeStream);
	//	    return;
	//	}

	//	if (dwOutput != fwrite(aOutput, sizeof(byte), dwOutput, m_hFile))
	//	{
	//	    beCloseStream(m_hbeStream);
	//	    return;
	//	}

	//	beCloseStream(m_hbeStream);

	//	if (m_beConfig.format.LHV1.bEnableVBR == TRUE)
	//	{
	//	    beWriteVBRHeader(m_fileName.c_str());
	//	}
	//}

}