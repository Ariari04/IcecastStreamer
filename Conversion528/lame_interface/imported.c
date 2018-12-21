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