#ifndef faad_h
#define faad_h

#ifdef WIN32
#include <tchar.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 */
typedef struct{
	char channelMode;   /* channel mode (mono:0/stereo:1) */
	int bitRate;       /* bit rate (kbit/s) */
	int samplingRate;  /* sampling rate (44.1 kHz -> 44) */
	int bitspersample;
	long duration;	  /* duration in milliseconds */
	char   title[128];
	char   artist[128];
	char   desc[128];
} aacheaderInfo;

#ifdef WIN32
	typedef TCHAR FAADCHAR;
#else
	typedef char FAADCHAR;
#endif


//return -1 if error
void* FAADInit(const FAADCHAR *szFilename, aacheaderInfo *setupInfo);
void FAADUnInit(void* hFile);

//return -1 if error
int FAADGetNextSample(void* hFile, char **pOut, int *cbStreamSize, long *nDecodeTimeSec);
void FAADFree(char *pbBuffer);

//return the actual position
long FAADSeek(void* hFile, int nMilliSec);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
