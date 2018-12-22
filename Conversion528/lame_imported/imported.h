#pragma once

#include <lame.h>

#if defined(__cplusplus)
extern "C" {
#endif

	int open_decoding(lame_t gf, FILE** outf);
	int open_encoding(lame_t gf, FILE** outf, size_t*  id3v2_size);

	int lame_main_imported(lame_t gf, int argc, char **argv, FILE** outf);
	int lame_decoder_iter(lame_t gfp, FILE * outf, char* Buffer, size_t Size, double* wavsize);
	int lame_encoder_iter(lame_t gfp, FILE * outf, char* Buffer, size_t Size, size_t  id3v2_size);

	int lame_decoding_close();
	int lame_encoding_close(FILE* outFile);

#if defined(__cplusplus)
}
#endif