#include <imported.h>

#include <stdio.h>
#include <windows.h>

#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif

#include "lame.h"

#define _O_BINARY      0x8000  // file mode is binary (untranslated)

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

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
	FILE* fh = 0;
	wchar_t* wfile = utf8ToUnicode(file);
	wchar_t* wmode = utf8ToUnicode(mode);
	if (wfile != 0 && wmode != 0) {
		fh = _wfopen(wfile, wmode);
	}
	else {
		fh = fopen(file, mode);
	}
	free(wfile);
	free(wmode);
	return fh;
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