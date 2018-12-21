#include <imported.h>

#include <assert.h>
#include <stdio.h>
#include <windows.h>

#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif

#include <lame.h>
#include <parse_imported.h>

#define _O_BINARY      0x8000  // file mode is binary (untranslated)

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

FILE * init_files(lame_global_flags * gf, char const *inPath, char const *outPath)
{
	FILE   *outf;
	/* Mostly it is not useful to use the same input and output name.
	   This test is very easy and buggy and don't recognize different names
	   assigning the same file
	 */
	if (0 != strcmp("-", outPath) && 0 == strcmp(inPath, outPath)) {
		error_printf("Input file and Output file are the same. Abort.\n");
		return NULL;
	}

	/* open the wav/aiff/raw pcm or mp3 input file.  This call will
	 * open the file, try to parse the headers and
	 * set gf.samplerate, gf.num_channels, gf.num_samples.
	 * if you want to do your own file input, skip this call and set
	 * samplerate, num_channels and num_samples yourself.
	 */
	if (init_infile(gf, inPath) < 0) {
		error_printf("Can't init infile '%s'\n", inPath);
		return NULL;
	}
	if ((outf = init_outfile(outPath, lame_get_decode_only(gf))) == NULL) {
		error_printf("Can't init outfile '%s'\n", outPath);
		return NULL;
	}

	return outf;
}

int lame_set_stream_binary_mode(FILE * const fp)
{
#if   defined __EMX__
	_fsetmode(fp, "b");
#elif defined __BORLANDC__
	setmode(_fileno(fp), O_BINARY);
#elif defined __CYGWIN__
	setmode(fileno(fp), _O_BINARY);
#elif defined _WIN32
	_setmode(_fileno(fp), _O_BINARY);
#else
	(void)fp;          /* doing nothing here, silencing the compiler only. */
#endif
	return 0;
}

void dosToLongFileName(char *fn)
{
	const size_t MSIZE = PATH_MAX + 1 - 4; /*  we wanna add ".mp3" later */
	WIN32_FIND_DATAA lpFindFileData;
	HANDLE  h = FindFirstFileA(fn, &lpFindFileData);
	if (h != INVALID_HANDLE_VALUE) {
		size_t  a;
		char   *q, *p;
		FindClose(h);
		for (a = 0; a < MSIZE; a++) {
			if ('\0' == lpFindFileData.cFileName[a])
				break;
		}
		if (a >= MSIZE || a == 0)
			return;
		q = strrchr(fn, '\\');
		p = strrchr(fn, '/');
		if (p - q > 0)
			q = p;
		if (q == NULL)
			q = strrchr(fn, ':');
		if (q == NULL)
			strncpy(fn, lpFindFileData.cFileName, a);
		else {
			a += q - fn + 1;
			if (a >= MSIZE)
				return;
			strncpy(++q, lpFindFileData.cFileName, MSIZE - a);
		}
	}
}

static wchar_t *mbsToUnicode(const char *mbstr, int code_page)
{
	int n = MultiByteToWideChar(code_page, 0, mbstr, -1, NULL, 0);
	wchar_t* wstr = malloc(n * sizeof(wstr[0]));
	if (wstr != 0) {
		n = MultiByteToWideChar(code_page, 0, mbstr, -1, wstr, n);
		if (n == 0) {
			free(wstr);
			wstr = 0;
		}
	}
	return wstr;
}

static char *unicodeToMbs(const wchar_t *wstr, int code_page)
{
	int n = 1 + WideCharToMultiByte(code_page, 0, wstr, -1, 0, 0, 0, 0);
	char* mbstr = malloc(n * sizeof(mbstr[0]));
	if (mbstr != 0) {
		n = WideCharToMultiByte(code_page, 0, wstr, -1, mbstr, n, 0, 0);
		if (n == 0) {
			free(mbstr);
			mbstr = 0;
		}
	}
	return mbstr;
}

static char *unicodeToUtf8(const wchar_t *wstr)
{
	return unicodeToMbs(wstr, CP_UTF8);
}

static char* mbsToMbs(const char* str, int cp_from, int cp_to)
{
	wchar_t* wstr = mbsToUnicode(str, cp_from);
	if (wstr != 0) {
		char* local8bit = unicodeToMbs(wstr, cp_to);
		free(wstr);
		return local8bit;
	}
	return 0;
}

char* utf8ToLatin1(char const* str)
{
	return mbsToMbs(str, CP_UTF8, 28591); /* Latin-1 is code page 28591 */
}

static wchar_t *utf8ToUnicode(const char *mbstr)
{
	return mbsToUnicode(mbstr, CP_UTF8);
}

unsigned short* utf8ToUtf16(char const* mbstr) /* additional Byte-Order-Marker */
{
	int n = MultiByteToWideChar(CP_UTF8, 0, mbstr, -1, NULL, 0);
	wchar_t* wstr = malloc((n + 1) * sizeof(wstr[0]));
	if (wstr != 0) {
		wstr[0] = 0xfeff; /* BOM */
		n = MultiByteToWideChar(CP_UTF8, 0, mbstr, -1, wstr + 1, n);
		if (n == 0) {
			free(wstr);
			wstr = 0;
		}
	}
	return wstr;
}

int write_xing_frame(lame_global_flags * gf, FILE * outf, size_t offset)
{
	unsigned char mp3buffer[LAME_MAXMP3BUFFER];
	size_t  imp3, owrite;

	imp3 = lame_get_lametag_frame(gf, mp3buffer, sizeof(mp3buffer));
	if (imp3 <= 0) {
		return 0;       /* nothing to do */
	}
	if (global_ui_config.silent <= 0) {
		console_printf("Writing LAME Tag...");
	}
	if (imp3 > sizeof(mp3buffer)) {
		error_printf
		("Error writing LAME-tag frame: buffer too small: buffer size=%d  frame size=%d\n",
			sizeof(mp3buffer), imp3);
		return -1;
	}
	assert(offset <= LONG_MAX);
	if (fseek(outf, (long)offset, SEEK_SET) != 0) {
		error_printf("fatal error: can't update LAME-tag frame!\n");
		return -1;
	}
	owrite = fwrite(mp3buffer, 1, imp3, outf);
	if (owrite != imp3) {
		error_printf("Error writing LAME-tag \n");
		return -1;
	}
	if (global_ui_config.silent <= 0) {
		console_printf("done\n");
	}
	assert(imp3 <= INT_MAX);
	return (int)imp3;
}

int write_id3v1_tag(lame_t gf, FILE * outf)
{
	unsigned char mp3buffer[128];
	size_t  imp3, owrite;

	imp3 = lame_get_id3v1_tag(gf, mp3buffer, sizeof(mp3buffer));
	if (imp3 <= 0) {
		return 0;
	}
	if (imp3 > sizeof(mp3buffer)) {
		error_printf("Error writing ID3v1 tag: buffer too small: buffer size=%d  ID3v1 size=%d\n",
			sizeof(mp3buffer), imp3);
		return 0;       /* not critical */
	}
	owrite = fwrite(mp3buffer, 1, imp3, outf);
	if (owrite != imp3) {
		error_printf("Error writing ID3v1 tag \n");
		return 1;
	}
	return 0;
}

#if defined( _WIN32 ) && !defined(__MINGW32__)

char* lame_getenv(char const* var)
{
	char* str = 0;
	wchar_t* wvar = utf8ToUnicode(var);
	if (wvar != 0) {
		wchar_t* wstr = _wgetenv(wvar);
		if (wstr != 0) {
			str = unicodeToUtf8(wstr);
		}
	}
	free(wvar);
	return str;
}

FILE* lame_fopen(char const* file, char const* mode)
{
	return fopen(file, mode);
	//FILE* fh = 0;
	//wchar_t* wfile = utf8ToUnicode(file);
	//wchar_t* wmode = utf8ToUnicode(mode);
	//if (wfile != 0 && wmode != 0) {
	//	fh = _wfopen(wfile, wmode);
	//}
	//else {
	//	fh = fopen(file, mode);
	//}
	//free(wfile);
	//free(wmode);
	//return fh;
}

#else

char* lame_getenv(char const* var)
{
	char* str = getenv(var);
	if (str) {
		return strdup(str);
	}
	return 0;
}

FILE* lame_fopen(char const* file, char const* mode)
{
	return fopen(file, mode);
}

#endif

#if defined(WIN32)

static BOOL SetPriorityClassMacro(DWORD p)
{
	HANDLE  op = GetCurrentProcess();
	return SetPriorityClass(op, p);
}

void setProcessPriority(int Priority)
{
	switch (Priority) {
	case 0:
	case 1:
		SetPriorityClassMacro(IDLE_PRIORITY_CLASS);
		console_printf("==> Priority set to Low.\n");
		break;
	default:
	case 2:
		SetPriorityClassMacro(NORMAL_PRIORITY_CLASS);
		console_printf("==> Priority set to Normal.\n");
		break;
	case 3:
	case 4:
		SetPriorityClassMacro(HIGH_PRIORITY_CLASS);
		console_printf("==> Priority set to High.\n");
		break;
	}
}
#endif


int
lame_decoder_iter(lame_t gfp, FILE * outf)
{
	short int Buffer[2][1152];
	int     i, iread;
	static double  wavsize = 0;
	int     tmp_num_channels = lame_get_num_channels(gfp);

	/* unknown size, so write maximum 32 bit signed value */

	iread = get_audio16(gfp, Buffer); /* read in 'iread' samples */
	if (iread >= 0) {
		wavsize += iread;
		put_audio16(outf, Buffer, iread, tmp_num_channels);
	}

	if (iread <= 0)
	{
		i = (16 / 8) * tmp_num_channels;
		assert(i > 0);
		if (wavsize <= 0) {
			if (global_ui_config.silent < 10)
				error_printf("WAVE file contains 0 PCM samples\n");
			wavsize = 0;
		}
		else if (wavsize > 0xFFFFFFD0 / i) {
			if (global_ui_config.silent < 10)
				error_printf("Very huge WAVE file, can't set filesize accordingly\n");
			wavsize = 0xFFFFFFD0;
		}
		else {
			wavsize *= i;
		}
		/* if outf is seekable, rewind and adjust length */
		if (!global_decoder.disable_wav_header && !fseek(outf, 0l, SEEK_SET))
			WriteWaveHeader(outf, (int)wavsize, lame_get_in_samplerate(gfp), tmp_num_channels, 16);
	}

	return iread;
}

int
lame_main(lame_t gf, int argc, char **argv, FILE** outf)
{
	char    inPath[PATH_MAX + 1];
	char    outPath[PATH_MAX + 1];
	char    nogapdir[PATH_MAX + 1];
	/* support for "nogap" encoding of up to 200 .wav files */
#define MAX_NOGAP 200
	int     nogapout = 0;
	int     max_nogap = MAX_NOGAP;
	char    nogap_inPath_[MAX_NOGAP][PATH_MAX + 1];
	char   *nogap_inPath[MAX_NOGAP];
	char    nogap_outPath_[MAX_NOGAP][PATH_MAX + 1];
	char   *nogap_outPath[MAX_NOGAP];

	int     ret;
	int     i;
	*outf = NULL;

	//lame_set_msgf(gf, &frontend_msgf);
	//lame_set_errorf(gf, &frontend_errorf);
	//lame_set_debugf(gf, &frontend_debugf);
	if (argc <= 1) {
		usage(stderr, argv[0]); /* no command-line args, print usage, exit  */
		return 1;
	}

	memset(inPath, 0, sizeof(inPath));
	memset(nogap_inPath_, 0, sizeof(nogap_inPath_));
	for (i = 0; i < MAX_NOGAP; ++i) {
		nogap_inPath[i] = &nogap_inPath_[i][0];
	}
	memset(nogap_outPath_, 0, sizeof(nogap_outPath_));
	for (i = 0; i < MAX_NOGAP; ++i) {
		nogap_outPath[i] = &nogap_outPath_[i][0];
	}

	/* parse the command line arguments, setting various flags in the
	 * struct 'gf'.  If you want to parse your own arguments,
	 * or call libmp3lame from a program which uses a GUI to set arguments,
	 * skip this call and set the values of interest in the gf struct.
	 * (see the file API and lame.h for documentation about these parameters)
	 */
	ret = parse_args(gf, argc, argv, inPath, outPath, nogap_inPath, &max_nogap);
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

	/* initialize input file.  This also sets samplerate and as much
	   other data on the input file as available in the headers */
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
		*outf = init_files(gf, nogap_inPath[0], nogap_outPath[0]);
	}
	else {
		*outf = init_files(gf, inPath, outPath);
	}
	if (*outf == NULL) {
		close_infile();
		return -1;
	}
	/* turn off automatic writing of ID3 tag data into mp3 stream
	 * we have to call it before 'lame_init_params', because that
	 * function would spit out ID3v2 tag data.
	 */
	lame_set_write_id3tag_automatic(gf, 0);

	/* Now that all the options are set, lame needs to analyze them and
	 * set some more internal options and check for problems
	 */
	ret = lame_init_params(gf);
	if (ret < 0) {
		if (ret == -1) {
			display_bitrates(stderr);
		}
		error_printf("fatal error during initialization\n");
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

	if (0 == global_decoder.disable_wav_header)
		WriteWaveHeader(*outf, 0x7FFFFFFF, lame_get_in_samplerate(gf), tmp_num_channels, 16);

	return 0;
}