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

FILE* lame_encoding_open(lame_global_flags * gf, char *outPath)
{
	FILE* outf;

	if (outf = init_outfile(gf, lame_get_decode_only(gf)) == NULL) {
		fprintf(stdout, "Can't init infile '%s'\n", outPath);
		return -1;
	}

	int ret = lame_init_params(gf);
	if (ret < 0) {
		if (ret == -1) {
			display_bitrates(stdout);
		}
		fprintf(stdout, "fatal error during initialization\n");
		close_outfile(outf);
		return ret;
	}

	return outf;
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

int lame_encoding_write(lame_global_flags * gf, int Buffer[2][1152], int iread, FILE * outf, int writeHeader)
{
	unsigned char mp3buffer[LAME_MAXMP3BUFFER];

	size_t id3v2_size = lame_get_id3v2_tag(gf, 0, 0);

	if (writeHeader)
	{
		if (id3v2_size > 0) {
			unsigned char *id3v2tag = malloc(id3v2_size);
			if (id3v2tag != 0) {
				size_t  n_bytes = lame_get_id3v2_tag(gf, id3v2tag, id3v2_size);
				size_t  written = fwrite(id3v2tag, 1, n_bytes, outf);
				free(id3v2tag);
				if (written != n_bytes) {
					error_printf("Error writing ID3v2 tag \n");
					return 1;
				}
			}
		}
		else {
			unsigned char* id3v2tag = getOldTag(gf);
			id3v2_size = sizeOfOldTag(gf);
			if (id3v2_size > 0) {
				size_t owrite = fwrite(id3v2tag, 1, id3v2_size, outf);
				if (owrite != id3v2_size) {
					error_printf("Error writing ID3v2 tag \n");
					return 1;
				}
			}
		}
		if (global_writer.flush_write == 1) {
			fflush(outf);
		}
	}

	int in_limit = lame_get_maximum_number_of_samples(gf, sizeof(mp3buffer));
	if (in_limit < 1)
		in_limit = 1;

	int owrite, imp3 = 0;

	int rest;

	if (iread >= 0) {
		const int* buffer_l = Buffer[0];
		const int* buffer_r = Buffer[1];
		int     rest = iread;
		
		// ITERATION

		int const chunk = rest < in_limit ? rest : in_limit;

		/* encode */

		imp3 = lame_encode_buffer_int(gf, buffer_l, buffer_r, chunk,
			mp3buffer, sizeof(mp3buffer));
		buffer_l += chunk;
		buffer_r += chunk;
		rest -= chunk;

		/* was our output buffer big enough? */
		if (imp3 < 0) {
			if (imp3 == -1)
				error_printf("mp3 buffer is not big enough... \n");
			else
				error_printf("mp3 internal error:  error code=%i\n", imp3);
			return 1;
		}
		owrite = (int)fwrite(mp3buffer, 1, imp3, outf);
		if (owrite != imp3) {
			error_printf("Error writing mp3 output \n");
			return 1;
		}

	}

	if (rest > 0)
	{
		return 0;
	}

	int nogap = 0;
	if (nogap)
		imp3 = lame_encode_flush_nogap(gf, mp3buffer, sizeof(mp3buffer)); /* may return one more mp3 frame */
	else
		imp3 = lame_encode_flush(gf, mp3buffer, sizeof(mp3buffer)); /* may return one more mp3 frame */

	if (imp3 < 0) {
		if (imp3 == -1)
			error_printf("mp3 buffer is not big enough... \n");
		else
			error_printf("mp3 internal error:  error code=%i\n", imp3);
		return 1;

	}

	owrite = (int)fwrite(mp3buffer, 1, imp3, outf);
	if (owrite != imp3) {
		error_printf("Error writing mp3 output \n");
		return 1;
	}
	if (global_writer.flush_write == 1) {
		fflush(outf);
	}
	imp3 = write_id3v1_tag(gf, outf);
	if (global_writer.flush_write == 1) {
		fflush(outf);
	}
	if (imp3) {
		return 1;
	}
	write_xing_frame(gf, outf, id3v2_size);
	if (global_writer.flush_write == 1) {
		fflush(outf);
	}
	//if (global_ui_config.silent <= 0) {
	//	print_trailing_info(gf);
	//}

	return 0;
}

int lame_decoding_close()
{
	close_infile();

	return 0;
}

int lame_encoding_close()
{
	close_outfile();

	return 0;
}