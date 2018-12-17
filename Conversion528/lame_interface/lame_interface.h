#pragma once

#include <lame.h>

#if defined(__cplusplus)
extern "C" {
#endif

	int lame_decoding_open(lame_global_flags * gf, char *inPath);
	FILE* lame_encoding_open(lame_global_flags * gf, char *outPath);

	int lame_decoding_read(lame_global_flags * gf, int Buffer[2][1152]);
	int lame_encoding_write(lame_global_flags * gf, int Buffer[2][1152], int iread, FILE * outf, int writeHeader);

	int lame_decoding_close();
	int lame_encoding_close();

#if defined(__cplusplus)
}
#endif