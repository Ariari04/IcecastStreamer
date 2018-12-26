/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003-2004 M. Bakker, Ahead Software AG, http://www.nero.com
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Ahead Software through Mpeg4AAClicense@nero.com.
**
** $Id: main.c,v 1.70 2004/01/06 11:59:47 menno Exp $
**/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//#define off_t __int64
#else
#include <time.h>
#define __int64 long long int
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> /* for stat() */
#include <string.h>

#include <faad.h>
#include "../mp4ff/mp4ff.h"

#include "interface.h"

#ifndef min
#define min(a,b) ( (a) < (b) ? (a) : (b) )
#endif

#define MAX_CHANNELS 6 /* make this higher to support files with
                          more channels */


int g_nAacStartPos = 0;
long g_nAacFileSize = 0;
long g_nAacDuration = 0;

/* FAAD file buffering routines */
typedef struct {
    long bytes_into_buffer;
    long bytes_consumed;
    long file_offset;
    unsigned char *buffer;
    int at_eof;
    FILE *infile;
} aac_buffer;

typedef struct {
    FILE				*hMP4File;
    int					mp4file;
	mp4ff_t				*infile;
	int					track;
	mp4ff_callback_t	*mp4cb;
	faacDecHandle		hDecoder;
	faacDecConfigurationPtr config;
    long				sampleId;
	long				numSamples;
	aac_buffer			b;

	char channelMode;   /* channel mode (mono:0/stereo:1) */
	int samplingRate;  /* sampling rate (44.1 kHz -> 44) */
	int bitspersample;

	long nCurPos;
} FAADFILE;



int fill_aacbuffer(aac_buffer *b)
{
    int bread;

    if (b->bytes_consumed > 0)
    {
		// 10.09.2008 Bugfix: crash on big ID3 headers
        if (b->bytes_into_buffer > 0)
        {
            memmove((void*)b->buffer, (void*)(b->buffer + b->bytes_consumed),
                b->bytes_into_buffer * sizeof(unsigned char));
        }

		// 10.09.2008 Bugfix: crash on big ID3 headers
		if (b->bytes_into_buffer < 0)
		{
			fseek(b->infile, -b->bytes_into_buffer, SEEK_CUR);
			b->bytes_consumed += b->bytes_into_buffer;
			b->bytes_into_buffer = 0;
		}

        if (!b->at_eof)
        {
            bread = fread((void*)(b->buffer + b->bytes_into_buffer), 1,
                b->bytes_consumed, b->infile);

            if (bread != b->bytes_consumed)
                b->at_eof = 1;

            b->bytes_into_buffer += bread;
        }

        b->bytes_consumed = 0;

        if (b->bytes_into_buffer > 3)
        {
            if (memcmp(b->buffer, "TAG", 3) == 0)
                b->bytes_into_buffer = 0;
        }
        if (b->bytes_into_buffer > 11)
        {
            if (memcmp(b->buffer, "LYRICSBEGIN", 11) == 0)
                b->bytes_into_buffer = 0;
        }
        if (b->bytes_into_buffer > 8)
        {
            if (memcmp(b->buffer, "APETAGEX", 8) == 0)
                b->bytes_into_buffer = 0;
        }
    }

    return 1;
}

void advance_buffer(aac_buffer *b, int bytes)
{
    b->file_offset += bytes;
    b->bytes_consumed = bytes;
    b->bytes_into_buffer -= bytes;
}

static int adts_sample_rates[] = {96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,7350,0,0,0};

int adts_parse(aac_buffer *b, int *bitrate, float *length)
{
    int frames, frame_length;
    int t_framelength = 0;
    int samplerate;
    float frames_per_sec, bytes_per_frame;

    /* Read all frames to ensure correct time and bitrate */
    for (frames = 0; /* */; frames++)
    {
        fill_aacbuffer(b);

        if (b->bytes_into_buffer > 7)
        {
            /* check syncword */
            if (!((b->buffer[0] == 0xFF)&&((b->buffer[1] & 0xF6) == 0xF0)))
                break;

            if (frames == 0)
                samplerate = adts_sample_rates[(b->buffer[2]&0x3c)>>2];

            frame_length = ((((unsigned int)b->buffer[3] & 0x3)) << 11)
                | (((unsigned int)b->buffer[4]) << 3) | (b->buffer[5] >> 5);

            t_framelength += frame_length;

            if (frame_length > b->bytes_into_buffer)
                break;

            advance_buffer(b, frame_length);
        } else {
            break;
        }
    }

    frames_per_sec = (float)samplerate/1024.0f;
    if (frames != 0)
        bytes_per_frame = (float)t_framelength/(float)(frames*1000);
    else
        bytes_per_frame = 0;
    *bitrate = (int)(8. * bytes_per_frame * frames_per_sec + 0.5);
    if (frames_per_sec != 0)
        *length = (float)frames/frames_per_sec;
    else
        *length = 1;

    return 1;
}



uint32_t read_callback(void *user_data, void *buffer, uint32_t length)
{
    return fread(buffer, 1, length, (FILE*)user_data);
}

uint32_t seek_callback(void *user_data, uint64_t position)
{
    return fseek((FILE*)user_data, position, SEEK_SET);
}

/* MicroSoft channel definitions */
#define SPEAKER_FRONT_LEFT             0x1
#define SPEAKER_FRONT_RIGHT            0x2
#define SPEAKER_FRONT_CENTER           0x4
#define SPEAKER_LOW_FREQUENCY          0x8
#define SPEAKER_BACK_LEFT              0x10
#define SPEAKER_BACK_RIGHT             0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER   0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER  0x80
#define SPEAKER_BACK_CENTER            0x100
#define SPEAKER_SIDE_LEFT              0x200
#define SPEAKER_SIDE_RIGHT             0x400
#define SPEAKER_TOP_CENTER             0x800
#define SPEAKER_TOP_FRONT_LEFT         0x1000
#define SPEAKER_TOP_FRONT_CENTER       0x2000
#define SPEAKER_TOP_FRONT_RIGHT        0x4000
#define SPEAKER_TOP_BACK_LEFT          0x8000
#define SPEAKER_TOP_BACK_CENTER        0x10000
#define SPEAKER_TOP_BACK_RIGHT         0x20000
#define SPEAKER_RESERVED               0x80000000


int decodeAACfile(FAADFILE *faad, char **pOut, int *cbStreamSize, long *nDecodeTimeSec)
{
    faacDecFrameInfo frameInfo;
	int nBytesPerSec;
	int i;

	do
	{
		*pOut = faacDecDecode(faad->hDecoder, &frameInfo, faad->b.buffer, faad->b.bytes_into_buffer);

		if (*pOut == NULL)
		{
			for (i = 0; i < 100; i++)
			{
				if (!faad->b.at_eof)
				{
					fseek(faad->b.infile, 10, SEEK_CUR);
					faad->b.bytes_consumed = faad->b.bytes_into_buffer;
					faad->b.bytes_into_buffer = 0;
					faad->b.file_offset += 10;

					fill_aacbuffer(&faad->b);

					*pOut = faacDecDecode(faad->hDecoder, &frameInfo, faad->b.buffer, faad->b.bytes_into_buffer);

					if (*pOut != NULL)
					{
						break;
					}
				}
			}

			if (*pOut == NULL)
			{
				return -1;
			}
		}
	}
	while (frameInfo.samples == 0);

    // update buffer indices
    advance_buffer(&faad->b, frameInfo.bytesconsumed);

    // fill buffer
    fill_aacbuffer(&faad->b);

    if (faad->b.bytes_into_buffer == 0)
	{
        *pOut = NULL; // to make sure it stops now
	}

	if(*pOut == NULL)
	{
		return -1;
	}

	*cbStreamSize = frameInfo.samples * frameInfo.channels;

	//bytes per second
	nBytesPerSec = faad->samplingRate * faad->channelMode * faad->bitspersample / 8;
	*nDecodeTimeSec = faad->nCurPos / nBytesPerSec;

	faad->nCurPos += *cbStreamSize;

	return 1;
}

int GetAACTrack(mp4ff_t *infile)
{
    /* find AAC track */
    int i, rc;
    int numTracks = mp4ff_total_tracks(infile);

    for (i = 0; i < numTracks; i++)
    {
        unsigned char *buff = NULL;
        unsigned int buff_size = 0;
        mp4AudioSpecificConfig mp4ASC;

        mp4ff_get_decoder_config(infile, i, &buff, &buff_size);

        if (buff)
        {
            rc = AudioSpecificConfig(buff, buff_size, &mp4ASC);
            free(buff);

            if (rc < 0)
                continue;
            return i;
        }
    }

    /* can't decode this */
    return -1;
}

unsigned long srates[] =
{
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000,
    12000, 11025, 8000
};

int decodeMP4file(FAADFILE *faad, char **pOut, int *cbStreamSize, long *nDecodeTimeSec)
{
    faacDecFrameInfo frameInfo;

 	unsigned char *buffer;
	unsigned int buffer_size;
    int rc;
    long dur;
	int nBytesPerSec;


    /* get acces unit from MP4 file */
    buffer = NULL;
    buffer_size = 0;

    dur = mp4ff_get_sample_duration(faad->infile, faad->track, faad->sampleId);
    rc = mp4ff_read_sample(faad->infile, faad->track, faad->sampleId, (char**)&buffer,  &buffer_size);
    if (rc == 0)
    {
        return -1;
    }

    *pOut = faacDecDecode(faad->hDecoder, &frameInfo, buffer, buffer_size);
    if (buffer)
	{
		free(buffer);
	}

	faad->sampleId++;

	*cbStreamSize = dur * 2 * frameInfo.channels ;

	//bytes per second
	nBytesPerSec = faad->samplingRate * faad->channelMode * faad->bitspersample / 8;
	*nDecodeTimeSec = faad->nCurPos / nBytesPerSec;

	faad->nCurPos += *cbStreamSize;


    return 1;
}

void* FAADInit(const FAADCHAR *szFilename, aacheaderInfo *setupInfo)
{
    unsigned char header[8];
	char *szStr;
	long samples;

 	FAADFILE *faad = malloc(sizeof(FAADFILE));
	memset(faad, 0, sizeof(FAADFILE));

	faad->nCurPos = 0;

	faad->hDecoder = faacDecOpen();

	/* Set the default object type and samplerate */
	/* This is useful for RAW AAC files */
	faad->config = faacDecGetCurrentConfiguration(faad->hDecoder);
	faad->config->defObjectType = LC;
	faad->config->outputFormat = FAAD_FMT_16BIT;
	faad->config->downMatrix = 0;
	faad->config->useOldADTSFormat = 0;
	faad->config->dontUpSampleImplicitSBR = 1;
	faacDecSetConfiguration(faad->hDecoder, faad->config);

	/* check for mp4 file */
    faad->mp4file = 0;

	faad->hMP4File = 0;

    	#ifdef WIN32
		faad->hMP4File = _tfopen(szFilename, _T("rb"));
	#else
		faad->hMP4File = fopen(szFilename, "rb"); // Unicode version is not ANSI compatible!
	#endif

    if (!faad->hMP4File)
    {
		free(faad);
		return (void*)0;
    }

    fread(header, 1, 8, faad->hMP4File);
    fseek(faad->hMP4File, 0, SEEK_SET);

    if (header[4] == 'f' && header[5] == 't' && header[6] == 'y' && header[7] == 'p')
        faad->mp4file = 1;

	faad->mp4cb = 0;
	faad->infile = 0;

	if(faad->mp4file)
	{
		float f = 1024.0;
		float seconds;
		unsigned char *buffer;
		unsigned int buffer_size;
		mp4AudioSpecificConfig mp4ASC;


		/* initialise the callback structure */
		faad->mp4cb = malloc(sizeof(mp4ff_callback_t));

		faad->mp4cb->read = read_callback;
		faad->mp4cb->seek = seek_callback;
		faad->mp4cb->user_data = faad->hMP4File;

		faad->infile = mp4ff_open_read(faad->mp4cb);
		if (!faad->infile)
		{
			free(faad->mp4cb);
			fclose(faad->hMP4File);
			free(faad);
			return 0;
		}

		if ((faad->track = GetAACTrack(faad->infile)) < 0)
		{
#ifdef WIN32
			OutputDebugString("Unable to find correct AAC sound track in the MP4 file.\n");
#endif

			free(faad->mp4cb);
			mp4ff_close(faad->infile);
			fclose(faad->hMP4File);
			free(faad);
			return 0;
		}

		buffer = NULL;
		buffer_size = 0;
		mp4ff_get_decoder_config(faad->infile, faad->track, &buffer, &buffer_size);

		if(faacDecInit2(faad->hDecoder, buffer, buffer_size,
						&setupInfo->samplingRate, &setupInfo->channelMode) < 0)
		{
			/* If some error initializing occured, skip the file */
			faacDecClose(faad->hDecoder);
			free(faad->mp4cb);
			mp4ff_close(faad->infile);
			fclose(faad->hMP4File);
			free(faad);
			free(buffer);
			return 0;
		}

		if (buffer)
		{
			AudioSpecificConfig(buffer, buffer_size, &mp4ASC);
			free(buffer);
		}

		setupInfo->channelMode = (char)mp4ff_get_channel_count(faad->infile, faad->track);
		setupInfo->bitRate = mp4ff_get_avg_bitrate(faad->infile, faad->track);
		setupInfo->samplingRate = mp4ff_get_sample_rate(faad->infile, faad->track);
		setupInfo->bitspersample = 16;

		faad->channelMode = setupInfo->channelMode;
		faad->samplingRate = setupInfo->samplingRate;
		faad->bitspersample = setupInfo->bitspersample;

	    samples = mp4ff_num_samples(faad->infile, faad->track);
        if (mp4ASC.sbr_present_flag == 1)
        {
            f = f * 2.0f;
        }
        seconds = (float)samples*(float)(f-1.0)/(float)setupInfo->samplingRate;
		setupInfo->duration = (long)(seconds * 1000);

		setupInfo->artist[0] = 0;
		mp4ff_meta_get_artist(faad->infile, &szStr);
		if(szStr)
		{
			strncpy(setupInfo->artist, szStr, sizeof(setupInfo->artist));
			free(szStr);
		}

		setupInfo->title[0] = 0;
		mp4ff_meta_get_title(faad->infile, &szStr);
		if(szStr)
		{
			strncpy(setupInfo->title, szStr, sizeof(setupInfo->title));
			free(szStr);
		}

		setupInfo->desc[0] = 0;
		mp4ff_meta_get_comment(faad->infile, &szStr);
		if(szStr)
		{
			strncpy(setupInfo->desc, szStr, sizeof(setupInfo->desc));
			free(szStr);
		}

		faad->numSamples = mp4ff_num_samples(faad->infile, faad->track);
		faad->sampleId = 0;
	}
	else
	{
		int tagsize;
		int bread, fileread;
		int header_type = 0;
		float length = 0;
		//struct stat filestats;
		struct stat statbuf;
		int res;

		memset(&faad->b, 0, sizeof(aac_buffer));

		faad->b.infile = faad->hMP4File;
		fseek(faad->b.infile, 0, SEEK_END);
		fileread = ftell(faad->b.infile);
		fseek(faad->b.infile, 0, SEEK_SET);

		if (!(faad->b.buffer = (unsigned char*)malloc(FAAD_MIN_STREAMSIZE*MAX_CHANNELS)))
		{
			fclose(faad->hMP4File);
			free(faad);
			return 0;
		}

		memset(faad->b.buffer, 0, FAAD_MIN_STREAMSIZE*MAX_CHANNELS);

		bread = fread(faad->b.buffer, 1, FAAD_MIN_STREAMSIZE*MAX_CHANNELS, faad->b.infile);
		faad->b.bytes_into_buffer = bread;
		faad->b.bytes_consumed = 0;
		faad->b.file_offset = 0;

		if (bread != FAAD_MIN_STREAMSIZE*MAX_CHANNELS)
			faad->b.at_eof = 1;

		tagsize = 0;
		if (!memcmp(faad->b.buffer, "ID3", 3))
		{
			/* high bit is not used */
			tagsize = (faad->b.buffer[6] << 21) | (faad->b.buffer[7] << 14) |
				(faad->b.buffer[8] <<  7) | (faad->b.buffer[9] <<  0);

			tagsize += 10;
			advance_buffer(&faad->b, tagsize);
			fill_aacbuffer(&faad->b);
		}

		/* get AAC infos for printing */
		header_type = 0;
		if ((faad->b.buffer[0] == 0xFF) && ((faad->b.buffer[1] & 0xF6) == 0xF0))
		{
			adts_parse(&faad->b, &setupInfo->bitRate, &length);
			fseek(faad->b.infile, tagsize, SEEK_SET);

			bread = fread(faad->b.buffer, 1, FAAD_MIN_STREAMSIZE*MAX_CHANNELS, faad->b.infile);
			if (bread != FAAD_MIN_STREAMSIZE*MAX_CHANNELS)
				faad->b.at_eof = 1;
			else
				faad->b.at_eof = 0;
			faad->b.bytes_into_buffer = bread;
			faad->b.bytes_consumed = 0;
			faad->b.file_offset = tagsize;

			header_type = 1;
		} else if (memcmp(faad->b.buffer, "ADIF", 4) == 0) {
			int skip_size = (faad->b.buffer[4] & 0x80) ? 9 : 0;
			setupInfo->bitRate = ((unsigned int)(faad->b.buffer[4 + skip_size] & 0x0F)<<19) |
				((unsigned int)faad->b.buffer[5 + skip_size]<<11) |
				((unsigned int)faad->b.buffer[6 + skip_size]<<3) |
				((unsigned int)faad->b.buffer[7 + skip_size] & 0xE0);

			length = (float)fileread;
			if (length != 0)
			{
				length = ((float)length*8.f)/((float)setupInfo->bitRate) + 0.5f;
			}

			header_type = 2;
		}

		fill_aacbuffer(&faad->b);
		if ((bread = faacDecInit(faad->hDecoder, faad->b.buffer,
			faad->b.bytes_into_buffer, &setupInfo->samplingRate, &setupInfo->channelMode)) < 0)
		{
			/* If some error initializing occured, skip the file */
			fprintf(stderr, "Error initializing decoder library.\n");
			if (faad->b.buffer)
				free(faad->b.buffer);
			faacDecClose(faad->hDecoder);
			fclose(faad->hMP4File);
			free(faad);
			return 0;
		}

		advance_buffer(&faad->b, bread);
		fill_aacbuffer(&faad->b);

		setupInfo->duration = (long)(length * 1000);

		faad->channelMode = setupInfo->channelMode;
		faad->samplingRate = setupInfo->samplingRate;
		faad->bitspersample = setupInfo->bitspersample = 16;

		setupInfo->artist[0] = 0;
		setupInfo->title[0] = 0;
		setupInfo->desc[0] = 0;

		g_nAacStartPos = faad->b.file_offset;
		g_nAacDuration = (long)(length * 1000);

        	#ifdef WIN32
        		res = _tstat(szFilename, &statbuf);
		#else
			res = stat(szFilename, &statbuf);
		#endif

		g_nAacFileSize = (long)statbuf.st_size;
	}

	return (void*)faad;
}

void FAADUnInit(void* hFile)
{
 	FAADFILE *faad = (FAADFILE*)hFile;

    faacDecClose(faad->hDecoder);
	if(faad->mp4cb)
	{
		free(faad->mp4cb);
	}
	if(faad->infile)
	{
		mp4ff_close(faad->infile);
	}

	if (faad->b.buffer)
	{
		free(faad->b.buffer);
	}

	fclose(faad->hMP4File);
	free(faad);
}

int FAADGetNextSample(void* hFile, char **pOut, int *cbStreamSize, long *nDecodeTimeSec)
{
	FAADFILE *faad = (FAADFILE*)hFile;

	if (faad->mp4file)
    {
		return decodeMP4file(faad, pOut, cbStreamSize, nDecodeTimeSec);
    } else {
        return decodeAACfile(faad, pOut, cbStreamSize, nDecodeTimeSec);
    }

	return 1;
}

void FAADFree(char *pbBuffer)
{
	free(pbBuffer);
}

long FAADSeek(void* hFile, int nMilliSec)
{
	FAADFILE *faad = (FAADFILE*)hFile;
    long dur;
	int nFrameSize;
	int nBytesPerSec;
	int nBytePos;

	//bytes per second
	nBytesPerSec = faad->samplingRate * faad->channelMode * faad->bitspersample / 8;

	//Byte position
	nBytePos = (int)((__int64)nMilliSec * (__int64)nBytesPerSec / 1000);

	if(faad->mp4file)
	{
		dur = mp4ff_get_sample_duration(faad->infile, faad->track, 0);
	}
	else
	{
		dur = 1024;
	}

	//any position must be aligned to this value
	nFrameSize = dur * faad->channelMode * faad->bitspersample / 8;

	//Align
	nBytePos /= nFrameSize;

	if(!faad->mp4file)
	{
		double dbAacNewPos = 0;

		dbAacNewPos = ( (double)(g_nAacFileSize - g_nAacStartPos) / g_nAacDuration ) * nMilliSec;


		fseek(faad->b.infile, g_nAacStartPos + (long)dbAacNewPos, SEEK_SET);
			faad->b.bytes_consumed += faad->b.bytes_into_buffer;
			faad->b.bytes_into_buffer = 0;
			faad->b.file_offset = g_nAacStartPos + (long)dbAacNewPos;

		fill_aacbuffer(&faad->b);
	}

	faad->sampleId = nBytePos;

	faacDecPostSeekReset(faad->hDecoder, faad->sampleId);

	faad->nCurPos = nBytePos * (__int64)nFrameSize;

	return (long)( (__int64)faad->sampleId * (__int64)nFrameSize * 1000 / nBytesPerSec);
}
