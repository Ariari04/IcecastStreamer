#pragma once

#include <lame.h>

#if defined(__cplusplus)
extern "C" {
#endif

	int lame_main(lame_t gf, int argc, char **argv, FILE** outf);
	int lame_decoder_iter(lame_t gfp, FILE * outf);

#if defined(__cplusplus)
}
#endif