#pragma once

#include <lame.h>

#ifdef __cplusplus
extern "C" {
#endif

	int lame_decoding_open(lame_global_flags * gf, char *inPath);

	int lame_decoding_read(lame_global_flags * gf, int Buffer[2][1152]);

	int lame_decoding_close();

#ifdef __cplusplus
}
#endif