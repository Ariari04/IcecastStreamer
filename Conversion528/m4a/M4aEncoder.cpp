//#include "Mp3Encoder.h"
//
//#include <string>
//
//#include <lame_imported/imported.h>
//#include <lame_imported/parse_imported.h>
//
//namespace Encoding
//{
//
//	Mp3Encoder::Mp3Encoder()
//	{
//		outFile = NULL;
//		gf = lame_init(); /* initialize libmp3lame */
//	}
//
//	Mp3Encoder::~Mp3Encoder()
//	{
//		if (outFile)
//		{
//			lame_encoding_close(outFile);
//			outFile = NULL;
//		}
//		lame_close(gf);
//	}
//
//	int Mp3Encoder::open(const char* fileName)
//	{
//		if (outFile)
//		{
//			lame_encoding_close(outFile);
//			outFile = NULL;
//		}
//
//		std::string additionalOutputFile = std::string(fileName) + ".mp3";
//		int argc = 4;
//		char* argv[4] = { "", "-r",  (char*)fileName, (char*)additionalOutputFile.c_str() };
//		if (lame_main_imported(gf, argc, argv, &outFile, 1))
//		{
//			return 0;
//		}
//
//		if (open_encoding(gf, &outFile, &id3v2_size))
//		{
//			return 0;
//		}
//
//		return true;
//	}
//
//	int Mp3Encoder::write(char* Buffer, size_t Count)
//	{
//		int iread = lame_encoder_iter(gf, outFile, Buffer, Count, id3v2_size);
//
//		if (iread < 1)
//		{
//			lame_decoding_close();
//			fclose(outFile);       /* close the output file */
//			outFile = NULL;
//			return 0;
//		}
//
//		return iread;
//	}
//
//}