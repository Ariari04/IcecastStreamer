#include "Mp3Encoder.h"

#include <exception>

#include <lame_interface/imported.h>
#include <lame_interface/parse_imported.h>
#include <lame_interface/lame_interface.h>

namespace Encoding
{

	Mp3Encoder::Mp3Encoder()
	{
		outFile = NULL;
		writeHeader = true;
		gf = lame_init(); /* initialize libmp3lame */
	}

	Mp3Encoder::~Mp3Encoder()
	{
		if (outFile)
		{
			lame_encoding_close(outFile);
			outFile = NULL;
		}
		lame_close(gf);
	}

	int Mp3Encoder::open(const char* fileName)
	{
		if (outFile)
		{
			lame_encoding_close(outFile);
			outFile = NULL;
		}

		int argc = 4;
		char* argv[4] = { "", "-r",  "test/raw.wav", "test/encoded.mp3" };
		if (lame_main_2(gf, argc, argv, &outFile))
		{
			return 1;
		}

		if (xxx_open_encode(gf, &outFile, &id3v2_size))
		{
			return 1;
		}

		return true;
	}

	int Mp3Encoder::open(const wchar_t* fileName)
	{
		std::exception("Not implemented");

		return false;
	}

	int Mp3Encoder::write(int Buffer[2][1152], size_t ElementSize, size_t Count, FILE* outFile)
	{
		int iread = lame_encoder_iter(gf, this->outFile, id3v2_size);

		if (iread < 1)
		{
			lame_decoding_close();
			fclose(this->outFile);       /* close the output file */
			this->outFile = NULL;
			return 0;
		}

		return iread;
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