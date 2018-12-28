#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

	int faad_open_decoding(int argc, char *argv[], int* mp4SampleCount);

	int faad_iteration_decoding(char* Buffer, size_t Size);

	int faad_close_decoding();

#if defined(__cplusplus)
}
#endif