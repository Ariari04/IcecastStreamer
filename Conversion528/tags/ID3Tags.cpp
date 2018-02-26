#include "ID3Tags.h"

#define ID3LIB_LINKOPTION 1
#include <id3/tag.h>
#include <id3/misc_support.h>

#include "../3rd/faad/faadecoder.h"
#include <cassert>


ID3_FrameID GetID3LibTagName(int nTag)
{
	switch(nTag)
	{
	case TAG_TITLE:
		return ID3FID_TITLE;
	case TAG_ARTIST:
		return ID3FID_LEADARTIST;
	case TAG_ALBUM:
		return ID3FID_ALBUM;
	case TAG_GENRE:
		return ID3FID_CONTENTTYPE;
	case TAG_COMMENT:
		return ID3FID_COMMENT;
	case TAG_TRACK:
		return ID3FID_TRACKNUM;
	case TAG_YEAR:
		return ID3FID_YEAR;
	case TAG_TITLE_2:
		return ID3FID_TITLE;
	case TAG_ARTIST_2:
		return ID3FID_LEADARTIST;
	case TAG_ALBUM_2:
		return ID3FID_ALBUM;
	case TAG_GENRE_2:
		return ID3FID_CONTENTTYPE;
	case TAG_COMMENT_2:
		return ID3FID_COMMENT;
	case TAG_TRACK_2:
		return ID3FID_TRACKNUM;
	case TAG_YEAR_2:
		return ID3FID_YEAR;
	case TAG_PUBLISHER:
		return ID3FID_PUBLISHER;
	case TAG_ORIGARTIST:
		return ID3FID_ORIGARTIST;
	case TAG_COPYRIGHT:
		return ID3FID_COPYRIGHT;
	case TAG_DISC:
		return ID3FID_PARTINSET;
	case TAG_URL:
		return ID3FID_WWWUSER;
	case TAG_COMPOSER:
		return  ID3FID_COMPOSER ;
	case TAG_BPM:
		return  ID3FID_BPM ;
	case TAG_ENCODED_BY:
		return ID3FID_ENCODEDBY;
	case TAG_ALBUM_ARTIST:
		return ID3FID_BAND;
	case TAG_RATING:
		return ID3FID_POPULARIMETER;
	default:
		return ID3FID_NOFRAME;
	}
}

unicode_t* ReverseEndian(unicode_t* pStr)
{
	for(int i = 0 ; pStr[i] != 0; ++i)
	{
		unicode_t tmp = (pStr[i] << 8) & 0xff00;
		tmp |= (pStr[i] >> 8) & 0x00ff;
		pStr[i] = tmp;
	}
	return pStr;
}

KString GetStringFromFrame(const ID3_Frame* pcFr, ID3_FieldID WantedId = ID3FN_TEXT)
{
	KString res ="";
	ID3_Frame::ConstIterator* it = pcFr->CreateIterator();
	const ID3_Field* f;
	while (NULL != (f = it->GetNext()))
	{
		ID3_TextEnc enc = f->GetEncoding();
		ID3_FieldID id = f->GetID();

		if(WantedId != id)
		{
			continue;
		}

		if (f->GetType() == ID3FTY_INTEGER)
		{
			res.Format(_T("%d"), f->Get());
			break;
		}
		else
		{
			switch(enc)
			{
				case ID3TE_ISO8859_1:
				{
					const size_t nSize = f->Size() + 1;
					char* pStr = new char[nSize];
					f->Get(pStr, nSize);
					res += ConvertLatin1ToKString(pStr);
					delete[] pStr;
				}
					break;
				case ID3TE_UTF16:
				case ID3TE_UTF16BE:
				{
					const size_t nSizeInbytes = f->Size();
					// length of string in characters + trailing zero
					const size_t nLengthOfString = nSizeInbytes/sizeof(unicode_t) + 1;
					unicode_t* pStr = new unicode_t[nLengthOfString];
					f->Get(pStr, nLengthOfString);
					pStr[nLengthOfString-1] = 0;
					if (enc == ID3TE_UTF16)
						pStr = ReverseEndian(pStr);
					res += ConvertUtf16ToKString(pStr);
					delete[] pStr;
				}
					break;
			}
		}
	}

	if (it)
	{
		delete it;
	}

	return res;
}

const char* Mp3ChannelModeToString(Mp3_ChannelMode mode)
{
	switch(mode)
	{
	case MP3CHANNELMODE_FALSE:
		return "False";
	case MP3CHANNELMODE_STEREO:
		return "Stereo";
	case MP3CHANNELMODE_JOINT_STEREO:
		return "Joint stereo";
	case MP3CHANNELMODE_DUAL_CHANNEL:
		return "Dual channel";
	case MP3CHANNELMODE_SINGLE_CHANNEL:
		return "Single channel";
	default:
		return "Unknown";
	};
}

const char* Mp3CRCToString(Mp3_Crc crc)
{
	switch(crc)
	{
	case MP3CRC_ERROR_SIZE:
		return "Error size";
	case MP3CRC_MISMATCH:
		return "Mismatch";
	case MP3CRC_NONE:
		return "None";
	case MP3CRC_OK:
		return "Ok";
	default:
		return "Unknown";
	};
}

const char* MpegLayerToString(Mpeg_Layers layer)
{
	switch(layer)
	{
	case MPEGLAYER_FALSE:
		return "False";
	case MPEGLAYER_UNDEFINED:
		return "Undefined";
	case MPEGLAYER_III:
		return "3";
	case MPEGLAYER_II:
		return "2";
	case MPEGLAYER_I:
		return "1";
	default:
		return "Unknown";
	}
}

const char* MpegVersioToString(Mpeg_Version version)
{
	switch(version)
	{
	case MPEGVERSION_FALSE:
		return "False";
	case MPEGVERSION_2_5:
		return "2.5";
	case MPEGVERSION_Reserved:
		return "Reserved";
	case MPEGVERSION_2:
		return "2";
	case MPEGVERSION_1:
		return "1";
	default:
		return "Unknown";
	}
}

TelError ReadTagsFromFileID3(const char* strFileName, KTags& rTags)
{
	ID3_Tag* pcTag = 0;
	rTags.ClearContent();

	for(int j = 0; j < 2; ++j)
	{
		if(!j)
		{
			pcTag = new ID3_Tag();
			pcTag->Link(strFileName, ID3TT_ID3V1);
		}
		else
		{
			if(pcTag)
			{
				delete pcTag;
				pcTag = 0;
			}

			pcTag = new ID3_Tag();
			pcTag->Link(strFileName, ID3TT_ID3V2);

			// Read file attributes
			const Mp3_Headerinfo* pMp3Info = pcTag->GetMp3HeaderInfo();
			if(pMp3Info)
			{
				/*
				KString strTmp;
				strTmp.Format(L"%d kbps",pMp3Info->bitrate/1000);
				rTags.m_cFileAttributes[FINFO_BITRATE] = strTmp;

				strTmp = Mp3ChannelModeToString(pMp3Info->channelmode);
				rTags.m_cFileAttributes[FINFO_CHANNELS] = strTmp;

				strTmp = pMp3Info->copyrighted ? L"Yes" : L"No";
				rTags.m_cFileAttributes[FINFO_COPYRIGHTED] = strTmp;

				strTmp = Mp3CRCToString(pMp3Info->crc);
				rTags.m_cFileAttributes[FINFO_CRC] = strTmp;

				strTmp.Format(L"%d",pMp3Info->frames);
				rTags.m_cFileAttributes[FINFO_FRAMES] = strTmp;

				strTmp.Format(L"%d",pMp3Info->framesize);
				rTags.m_cFileAttributes[FINFO_FRAME_SIZE] = strTmp;

				strTmp.Format(L"%d Hz",pMp3Info->frequency);
				rTags.m_cFileAttributes[FINFO_SAMPLE_RATE] = strTmp;

				strTmp = MpegLayerToString(pMp3Info->layer);
				rTags.m_cFileAttributes[FINFO_MPEG_LAYER] = strTmp;

				strTmp = pMp3Info->original ? L"Yes" : L"No";
				rTags.m_cFileAttributes[FINFO_ORIGINAL] = strTmp;

				strTmp = pMp3Info->privatebit ? L"Yes" : L"No";
				rTags.m_cFileAttributes[FINFO_PRIVATE_BIT] = strTmp;

				strTmp.Format(L"%02d:%02d:%02d", pMp3Info->time/3600, (pMp3Info->time/60)%60, pMp3Info->time%60);
				rTags.m_cFileAttributes[FINFO_DURATION] = strTmp;

				if (pMp3Info->vbr_bitrate)
				{
					strTmp.Format(L"%d", pMp3Info->vbr_bitrate);
					rTags.m_cFileAttributes[FINFO_AVG_VBR] = strTmp;
				}

				strTmp = MpegVersioToString(pMp3Info->version);
				rTags.m_cFileAttributes[FINFO_MPEG_VERSION] = strTmp;

				strTmp.Format(L"%1.3f MB", pcTag->GetFileSize()/(1024.0*1024.0));
				rTags.m_cFileAttributes[FINFO_FILE_SIZE] = strTmp;
				*/
				rTags.getAttributes().playTime = pMp3Info->time;
			}
			else // AAC file here
			{

				aacheaderInfo cInfo;
				void* hFile = FAADInit(strFileName, &cInfo);
				if (hFile)
					FAADUnInit(hFile);

				/*
				KString strTmp;
				strTmp.Format(L"%d kbps",cInfo.bitRate);
				rTags.m_cFileAttributes[FINFO_BITRATE] = strTmp;

				strTmp.Format(L"%d", cInfo.channelMode);
				rTags.m_cFileAttributes[FINFO_CHANNELS] = strTmp;

				int nDur = cInfo.duration /1000;
				strTmp.Format(L"%02d:%02d:%02d", nDur/3600, (nDur/60)%60, nDur%60);
				rTags.m_cFileAttributes[FINFO_DURATION] = strTmp;

				strTmp.Format(L"%d Hz",cInfo.samplingRate);
				rTags.m_cFileAttributes[FINFO_SAMPLE_RATE] = strTmp;

				strTmp.Format(L"%d",cInfo.bitspersample);
				rTags.m_cFileAttributes[FINFO_BITS_PER_SAMPLE] = strTmp;
				*/

				rTags.getAttributes().playTime = cInfo.duration /1000;
			}
		}


		for (int i = (j ? TAG_ID3V2_START : TAG_ID3V1_START);
			i < (j ? TAG_ID3V2_FIN : TAG_ID3V1_FIN); ++i)
		{
			ID3_Frame* pcFr = 0;
			pcFr = pcTag->Find(GetID3LibTagName(i));
			if (pcFr)
			{
				if (i == TAG_GENRE || i == TAG_GENRE_2 )
				{
					KString strNum = GetStringFromFrame(pcFr);
					if (strNum.GetLength())
					{
						KString strBkp = strNum;
						strNum = strNum.Right(strNum.GetLength()-1);
						strNum = strNum.Left(strNum.GetLength()-1);
						int nCompar = -1;

						if (strNum.GetLength() == 1)
						{
							nCompar = strNum.CompareNoCase("0");
						}

						unsigned int n = atoi(strNum);

						if( (n > 0 && n < GENRE_COUNT) || (nCompar == 0) )
						{
							rTags.SetTag(i, GENRE_NAMES[n]);
						}
						else
						{
							rTags.SetTag(i, strBkp);
						}
					}
				}
				else if (i == TAG_URL)
				{
					KString strUrl = GetStringFromFrame(pcFr, ID3FN_URL);
					rTags.SetTag(i,strUrl);
				}
				else if (i == TAG_RATING)
				{
					KString strRate = GetStringFromFrame(pcFr, ID3FN_RATING);
					rTags.SetTag(i, strRate);
				}
				else // i != TAG_GENRE && i != TAG_URL && i != TAG_RATING
				{
					KString strContent = GetStringFromFrame(pcFr);
					rTags.SetTag(i,strContent);
				}
			}
		}
	}

	if (pcTag)
	{
		delete pcTag;
		pcTag = 0;
	}

	rTags.SetID3VXFlags();

	return 0;
}

void SetStringToField(ID3_Field* pField, const KString& str, bool bWriteInASCII = false)
{
	assert(pField);

	if (pField->GetType() == ID3FTY_INTEGER)
	{
		pField->Set(atoi((const char*)str));
	}
	else if (bWriteInASCII)
	{
		pField->SetEncoding(ID3TE_ASCII);
		pField->Set((const char*)str);
	}
	else
	{
	#ifdef WIN32
		int n = str.GetLength();
		wchar_t* pStr = new wchar_t[n+1];
		wcscpy(pStr, (const wchar_t*)str);
		size_t nUsed;
		pField->SetEncoding(ID3TE_UTF16);
		nUsed = pField->Set(ReverseEndian((unicode_t*)pStr));
		delete pStr;
	#else
		int n = str.GetLength();
		unicode_t* pStr = new unicode_t[n+1];
		memcpy(pStr, str.GetAsUtf16(), 2*n);
		pStr[n] = unicode_t(0);
		size_t nUsed;
		pField->SetEncoding(ID3TE_UTF16);
		nUsed = pField->Set(ReverseEndian((unicode_t*)pStr));
		delete pStr;
	#endif
	}
}

void SetStringToFrame(ID3_Frame* pFr, const KString& str, ID3_FieldID field = ID3FN_TEXT, bool bWriteInASCII = false)
{
	ID3_Field* pF = pFr->GetField(ID3FN_TEXTENC);

	if (pF)
	{
		if (bWriteInASCII)
		{
			pF->Set(ID3TE_ASCII);
		}
		else
		{
			pF->Set(ID3TE_UTF16);
		}
	}

	SetStringToField(pFr->GetField(field), str, bWriteInASCII);
}


KString MakeGenreStringFromTag(const KString& strTag)
{
	KString strCurTag = strTag;
	for(unsigned int j = 0 ; j < GENRE_COUNT; ++j)
	{
		if(!strCurTag.CompareNoCase(GENRE_NAMES[j]))
		{
			KString strTmp;
			strTmp.Format(L"(%d)", j);
			return strTmp;
		}
	}
	return strCurTag;
}

TelError WriteTagsToFileID3(const char* strFileName,const KTags& rTagsToWrite)
{
	const KTags rTags = rTagsToWrite.MakeFullID3Copy();

	ID3_Frame* arrpFrames[TAG_FIN];

	for (int i = TAG_ID3V1_START; i < TAG_ID3V2_FIN; i++)
	{
		arrpFrames[i] = NULL;
	}

	ID3_Tag cTagV1;
	cTagV1.Link(strFileName, ID3TT_ID3V1);
	cTagV1.Clear();

	if(rTags.bHasID3v1Tags)
	{
		for(int i = TAG_ID3V1_START; i < TAG_ID3V1_FIN; ++i)
		{
			KString strCurTag;
			strCurTag = rTags.GetTag(i);

			if(!strCurTag.GetLength())
			{
				continue;
			}

			arrpFrames[i] = new ID3_Frame(GetID3LibTagName(i));
			ID3_FieldID nField = ID3FN_TEXT;

			if( i == TAG_GENRE)
			{
				SetStringToFrame(arrpFrames[i], MakeGenreStringFromTag(strCurTag), nField, true);
			}
			else
			{
				SetStringToFrame(arrpFrames[i], strCurTag, nField, true);
			}

			cTagV1.AddFrame(arrpFrames[i]);
		}//end of for loop body
	}

	flags_t tagsv1 = cTagV1.Update(ID3TT_ID3V1);

	ID3_Tag cTagV2;
	cTagV2.Link(strFileName, ID3TT_ID3V2);
	cTagV2.Clear();

	if (rTags.bHasID3v2Tags)
	{
		for(int i = TAG_ID3V2_START; i < TAG_ID3V2_FIN; ++i)
		{
			KString strCurTag;
			strCurTag = rTags.GetTag(i);

			if(!strCurTag.GetLength())
			{
				continue;
			}

			arrpFrames[i] = new ID3_Frame(GetID3LibTagName(i));
			ID3_FieldID nField;

			if (i == TAG_URL)
			{
				nField = ID3FN_URL;
			}
			else if (i == TAG_RATING)
			{
				nField = ID3FN_RATING;
			}
			else
			{
				nField = ID3FN_TEXT;
			}

			if (i == TAG_GENRE_2)
			{
				SetStringToFrame(arrpFrames[i], MakeGenreStringFromTag(strCurTag),
					nField);
			}
			else
			{
				SetStringToFrame(arrpFrames[i], strCurTag, nField);
			}

			cTagV2.AddFrame(arrpFrames[i]);
		}
	}

	//WritePicturesToFileID3(cTagV2, rTags.m_vPictures);

	flags_t tagsv2 = cTagV2.Update(ID3TT_ID3V2);

	for (int i = TAG_ID3V1_START; i < TAG_ID3V2_FIN; i++)
	{
		if (arrpFrames[i])
		{
			delete arrpFrames[i];
			arrpFrames[i] = NULL;
		}
	}

	if (tagsv2 == ID3TT_NONE || tagsv1 == ID3TT_NONE)
	{
		// no tags was written - return error
		return TEL_ERR_CANT_WRITE_FILE;
	}

	return TEL_ERR_OK;
}
