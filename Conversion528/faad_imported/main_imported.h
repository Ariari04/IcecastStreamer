#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

	int faad_open_decoding(int argc, char *argv[]);

	int decodeAacfile_iteration();
	int decodeMP4file_iteration();

	int decodeAacfile_closing();
	int decodeMP4file_closing();

#if defined(__cplusplus)
}
#endif