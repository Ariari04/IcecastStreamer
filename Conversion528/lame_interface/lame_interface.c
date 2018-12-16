#include <lame_interface.h>

#include <get_audio.h>
#include <parse_imported.h>
#include <string.h>

int lame_decoding_open(lame_global_flags * gf, char *inPath)
{
	if (init_infile(gf, inPath) < 0) {
		fprintf(stdout, "Can't init infile '%s'\n", inPath);
		return -1;
	}

	int ret = lame_init_params(gf);
	if (ret < 0) {
		if (ret == -1) {
			display_bitrates(stdout);
		}
		fprintf(stdout, "fatal error during initialization\n");
		close_infile();
		return ret;
	}

	return 0;
}

// Buffer is int Buffer[2][MP3_FRAME_BUFFER_SIZE]
int lame_decoding_read(lame_global_flags * gf, int Buffer[2][1152])
{
	memset(Buffer[0], 0, 1152 * sizeof(int));
	memset(Buffer[1], 0, 1152 * sizeof(int));

	/* read in 'iread' samples */
	int iread = get_audio(gf, Buffer);

	return iread;
}

int lame_decoding_close()
{
	close_infile();     /* close the input file */

	return 0;
}