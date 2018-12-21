#pragma once

#include <lame.h>

#if defined(__cplusplus)
extern "C" {
#endif

	int lame_main_2(lame_t gf, int argc, char **argv, FILE** outf);
	int lame_decoder_iter(lame_t gfp, FILE * outf);
	int lame_encoder_iter(lame_t gfp, FILE * outf, size_t  id3v2_size);

	int xxx_open_decode(lame_t gf, FILE** outf);
	int xxx_open_encode(lame_t gf, FILE** outf, size_t*  id3v2_size);

#if defined(__cplusplus)
}
#endif