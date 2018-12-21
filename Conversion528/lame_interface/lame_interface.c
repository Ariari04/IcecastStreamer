#include <lame_interface.h>

#include <get_audio_imported.h>
#include <parse_imported.h>
#include <string.h>

int lame_decoding_open(lame_global_flags * gf, char *inputFile, FILE   **outf)
{
	int     i;
	int argc = 4;
	const char* argv[4] = { "", "--decode", inputFile, "test/xxx.wav" };

	char inPath[PATH_MAX + 1];
	char outPath[PATH_MAX + 1];
	char    nogapdir[PATH_MAX + 1];

#define MAX_NOGAP 200

	int     nogapout = 0;
	int max_nogap = MAX_NOGAP;
	char    nogap_inPath_[MAX_NOGAP][PATH_MAX + 1];
	char   *nogap_inPath[MAX_NOGAP];
	char    nogap_outPath_[MAX_NOGAP][PATH_MAX + 1];
	char   *nogap_outPath[MAX_NOGAP];

	//lame_set_msgf(gf, &frontend_msgf);
	//lame_set_errorf(gf, &frontend_errorf);
	//lame_set_debugf(gf, &frontend_debugf);

	memset(inPath, 0, sizeof(inPath));
	memset(nogap_inPath_, 0, sizeof(nogap_inPath_));
	for (i = 0; i < MAX_NOGAP; ++i) {
		nogap_inPath[i] = &nogap_inPath_[i][0];
	}
	memset(nogap_outPath_, 0, sizeof(nogap_outPath_));
	for (i = 0; i < MAX_NOGAP; ++i) {
		nogap_outPath[i] = &nogap_outPath_[i][0];
	}

	int ret = parse_args(gf, argc, argv, inPath, outPath, nogap_inPath, &max_nogap);
	if (ret < 0) {
		return ret == -2 ? 0 : 1;
	}

	if (global_ui_config.update_interval < 0.)
		global_ui_config.update_interval = 2.;

	if (outPath[0] != '\0' && max_nogap > 0) {
		strncpy(nogapdir, outPath, PATH_MAX + 1);
		nogapdir[PATH_MAX] = '\0';
		nogapout = 1;
	}

	*outf = NULL;
	if (max_nogap > 0) {
		/* for nogap encoding of multiple input files, it is not possible to
		 * specify the output file name, only an optional output directory. */
		for (i = 0; i < max_nogap; ++i) {
			char const* outdir = nogapout ? nogapdir : "";
			if (generateOutPath(nogap_inPath[i], outdir, ".mp3", nogap_outPath[i]) != 0) {
				error_printf("processing nogap file %d: %s\n", i + 1, nogap_inPath[i]);
				return -1;
			}
		}
		*outf = init_files(gf, inPath, outPath);
	}
	else
	{
		*outf = init_files(gf, inPath, outPath);
	}
	if (*outf == NULL) {
		close_infile();
		return -1;
	}
    
    lame_set_write_id3tag_automatic(gf, 0);

	ret = lame_init_params(gf);
	if (ret < 0) {
		if (ret == -1) {
			display_bitrates(stdout);
		}
		fprintf(stdout, "fatal error during initialization\n");
		fclose(*outf);
		close_infile();
		return ret;
	}

	if (global_ui_config.silent > 0) {
		global_ui_config.brhist = 0; /* turn off VBR histogram */
	}

	int     tmp_num_channels = lame_get_num_channels(gf);

	if (!(tmp_num_channels >= 1 && tmp_num_channels <= 2)) {
		error_printf("Internal error.  Aborting.");
		return -1;
	}

	return 0;
}

FILE* lame_encoding_open(lame_global_flags * gf, char *outPath)
{
	init_infile(gf, ".mp3");

	FILE* outf = init_outfile(outPath, lame_get_decode_only(gf));

	if (outf == NULL) {
		fprintf(stdout, "Can't init infile '%s'\n", outPath);
		return -1;
	}

	int ret = lame_init_params(gf);
	if (ret < 0) {
		if (ret == -1) {
			display_bitrates(stdout);
		}
		fprintf(stdout, "fatal error during initialization\n");
		//close_outfile(outf);
		fclose(outf);
		return ret;
	}

	return outf;
}

int chn = 0;

// Buffer is int Buffer[2][MP3_FRAME_BUFFER_SIZE]
int lame_decoding_read(lame_global_flags * gf, char Buffer[2 * 1152 * 2])
{
	short int tmp[2][1152];

	memset(tmp[0], 0, 1152 * 2);
	memset(tmp[1], 0, 1152 * 2);

	//iread = get_audio16(gf, Buffer); /* read in 'iread' samples */
	int iread = get_audio16(gf, tmp); /* read in 'iread' samples */

	if (!chn ) chn = lame_get_num_channels(gf);

	if (iread)
	{
		iread = put_audio16_imported(tmp, Buffer, iread, chn);
	}

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
	close_infile();     /* close the input file */

	return 0;
}

int lame_encoding_close(FILE* outFile)
{
	if (!outFile)
	{
		//close_outfile(outFile);
		fclose(outFile);
	}

	return 0;
}