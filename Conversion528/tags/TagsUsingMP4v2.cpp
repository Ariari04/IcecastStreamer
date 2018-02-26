#include "../stdafx.h"
#include "TagsUsingMP4v2.h"
#include <mp4.h>
#include <cerrno>
#include "../3rd/faad/faadecoder.h"
#include "../PathOperations.h"

namespace
{

	template <typename _ExtractingFunction>
	void ExtractTagString(MP4FileHandle hFile, KTags& rTags, _ExtractingFunction func, int nTagId)
	{
		char * strTagData = 0;
		bool bRes = func(hFile, &strTagData);
		KString strTag = (bRes && strTagData) ? ConvertUtf8ToKString(strTagData) : KString("");
		free(strTagData);
		rTags.SetTag(nTagId, strTag);
	}

	template <typename _ExtractingFunction>
	void ExtractTagStringToMP4Tags(MP4FileHandle hFile, KTags& rTags, _ExtractingFunction func, int nTagId)
	{
		char * strTagData = 0;
		bool bRes = func(hFile, &strTagData);
		KString strTag = (bRes && strTagData) ? ConvertUtf8ToKString(strTagData) : KString("");
		free(strTagData);
		rTags.m_cMP4Tags[nTagId] = strTag;
	}

	template <typename _ExtractingFunction>
	void ExtractNumberOfTotal(MP4FileHandle hFile, KTags& rTags, _ExtractingFunction func, int nTagId)
	{
		unsigned short number(0);
		unsigned short total(0);
		bool bRes = func(hFile, &number, &total);
		KString strTag ="";
		if(bRes)
		{
			strTag.Format(L"%d/%d",number,total);
		}
		rTags.SetTag(nTagId, strTag);
	}

	template <typename _ExtractingFunction>
	void ExtractNumber(MP4FileHandle hFile, KTags& rTags, _ExtractingFunction func, int nTagId)
	{
		unsigned short number(0);
		bool bRes = func(hFile, &number);
		KString strTag ="";
		if(bRes)
		{
			strTag.Format(L"%d",number);
		}
		rTags.SetTag(nTagId, strTag);
	}

	typedef struct
	{
		unsigned short Number;
		unsigned short Total;
	} NumberOfTotal;

	inline unsigned short ParseNumber(const KString& str)
	{
		return atoi((const char*)str);
	}

	NumberOfTotal ParseNumberOfTotalString(const KString& str)
	{
		int nSep = str.Find(L'/');
		KString strTmp(str);
		NumberOfTotal res;
		strTmp = strTmp.Left(nSep);
		res.Number = ParseNumber(strTmp);
		strTmp = str;
		strTmp = strTmp.Right(strTmp.GetLength() - nSep - 1);
		res.Total  = ParseNumber(strTmp);
		return res;
	}

}//namespace


TelError ReadAttributesFromMP4(const char* strFileName, KTags::FileAttributes& rAttributes)
{
	// Read file attributes
	aacheaderInfo cInfo;
	void* hFile = FAADInit(strFileName, &cInfo);
	if (hFile == 0)
	{
		return TEL_ERR_CANT_OPEN_FILE;
	}

	FAADUnInit(hFile);

	/*
	KString strTmp;
	strTmp.Format(L"%d kbps",cInfo.bitRate/1000);
	rAttributes[FINFO_BITRATE] = strTmp;

	strTmp.Format(L"%d", cInfo.channelMode);
	rAttributes[FINFO_CHANNELS] = strTmp;

	int nDur = cInfo.duration /1000;
	strTmp.Format(L"%02d:%02d:%02d", nDur/3600, (nDur/60)%60, nDur%60);
	rAttributes[FINFO_DURATION] = strTmp;

	strTmp.Format(L"%d Hz",cInfo.samplingRate);
	rAttributes[FINFO_SAMPLE_RATE] = strTmp;

	strTmp.Format(L"%d",cInfo.bitspersample);
	rAttributes[FINFO_BITS_PER_SAMPLE] = strTmp;
	*/

	rAttributes.playTime = cInfo.duration / 1000;

	return TEL_ERR_OK;
}

TelError ReadTagsFromFileMP4(const char* strFileName, KTags& rTags)
{
	ReadAttributesFromMP4(strFileName, rTags.getAttributes());

	MP4FileHandle hFile = MP4Read(strFileName);
	if(hFile == MP4_INVALID_FILE_HANDLE)
	{
		return TEL_ERR_CANT_OPEN_FILE;
	}

	rTags.ClearContent();

	ExtractTagString(hFile, rTags, MP4GetMetadataName, TAG_TITLE);
	ExtractTagString(hFile, rTags, MP4GetMetadataAlbum, TAG_ALBUM);
	ExtractTagString(hFile, rTags, MP4GetMetadataArtist, TAG_ARTIST);
	ExtractTagString(hFile, rTags, MP4GetMetadataComment, TAG_COMMENT);
	ExtractTagString(hFile, rTags, MP4GetMetadataGenre, TAG_GENRE);
	ExtractTagString(hFile, rTags, MP4GetMetadataWriter, TAG_COMPOSER);
	ExtractTagString(hFile, rTags, MP4GetMetadataYear, TAG_YEAR);
	// there is no such function in libmp4v2-1.5
	//ExtractTagString(hFile, rTags, MP4GetMetadataAlbumArtist, TAG_ALBUM_ARTIST);

	ExtractTagStringToMP4Tags(hFile, rTags, MP4GetMetadataTool, TAG_MP4_TOOL);

	ExtractNumberOfTotal(hFile, rTags, MP4GetMetadataTrack, TAG_TRACK);
	ExtractNumberOfTotal(hFile, rTags, MP4GetMetadataDisk, TAG_DISC);

	ExtractNumber(hFile, rTags, MP4GetMetadataTempo, TAG_BPM);

	byte *pRating;
	uint32 nSize;
	bool bRes = MP4GetMetadataFreeForm(hFile, "POPM", (u_int8_t **)&pRating, &nSize);

	if (bRes)
	{
		KString strTemp;
		strTemp.Format(_T("%d"), pRating[0]);
		rTags.SetTag(TAG_RATING, strTemp);
	}

	rTags.CopyID3v1ToID3v2();
	rTags.SetID3VXFlags();

	MP4Close(hFile);
	return TEL_ERR_OK;
}


TelError WriteTagsToFileMP4(const char* strFileName, const KTags& rTags)
{
	MP4FileHandle hFileToRead = MP4Read(strFileName);
	if(hFileToRead == MP4_INVALID_FILE_HANDLE)
	{
		return TEL_ERR_CANT_OPEN_FILE;
	}

	std::string strTmpName = PathOperations::GenerateTmpName(strFileName);
	if(!strTmpName.size())
	{
		return TEL_ERR_INTERNAL;
	}
	MP4FileHandle hFileToWrite = MP4Create(strTmpName.c_str());
	if(hFileToWrite == MP4_INVALID_FILE_HANDLE)
	{
		return TEL_ERR_CANT_WRITE_FILE;
	}

	MP4CopyTrack(hFileToRead, MP4FindTrackId(hFileToRead,0,MP4_AUDIO_TRACK_TYPE), hFileToWrite);
	MP4Close(hFileToRead);

	MP4SetMetadataTool				(hFileToWrite, rTags.GetMP4Tag(TAG_MP4_TOOL).GetAsUtf8());
	// there is no such function in libmp4v2-1.5
	//MP4SetMetadataAlbumArtist		(hFileToWrite, rTags.GetTag(TAG_ALBUM_ARTIST).GetAsUtf8());
	MP4SetMetadataAlbum				(hFileToWrite, rTags.GetTag(TAG_ALBUM_2).GetAsUtf8());
	MP4SetMetadataArtist			(hFileToWrite, rTags.GetTag(TAG_ARTIST_2).GetAsUtf8());
	MP4SetMetadataComment			(hFileToWrite, rTags.GetTag(TAG_COMMENT_2).GetAsUtf8());
	MP4SetMetadataGenre				(hFileToWrite, rTags.GetTag(TAG_GENRE_2).GetAsUtf8());
	MP4SetMetadataName				(hFileToWrite, rTags.GetTag(TAG_TITLE_2).GetAsUtf8());
	MP4SetMetadataWriter			(hFileToWrite, rTags.GetTag(TAG_COMPOSER).GetAsUtf8());
	MP4SetMetadataYear				(hFileToWrite, rTags.GetTag(TAG_YEAR).GetAsUtf8());

	NumberOfTotal cNoT;
	cNoT = ParseNumberOfTotalString(rTags.GetTag(TAG_TRACK_2));
	MP4SetMetadataTrack				(hFileToWrite, cNoT.Number, cNoT.Total);
	cNoT = ParseNumberOfTotalString(rTags.GetTag(TAG_DISC));
	MP4SetMetadataDisk				(hFileToWrite, cNoT.Number, cNoT.Total);
	MP4SetMetadataTempo				(hFileToWrite, ParseNumber(rTags.GetTag(TAG_BPM)));

	if (rTags.GetTag(TAG_RATING).GetLength() > 0)
	{
		byte nRating = (byte)atoi((const char *)rTags.GetTag(TAG_RATING));
		if (errno != EINVAL)
		{
			MP4SetMetadataFreeForm			(hFileToWrite, "POPM", &nRating, 1);
		}
	}


	/*//Write one image
	if(rTags.m_vPictures.size())
	{
		const char* pData = 0;
		unsigned long nDataSize = 0;
		rTags.m_vPictures[0].GetPictureData(&pData, &nDataSize);
		MP4SetMetadataCoverArt(hFileToWrite,((const u_int8_t *)pData), nDataSize);
	}
	//End write one image*/

	MP4Close(hFileToWrite);

	remove(strFileName);

	if (0 != rename(strTmpName.c_str(), strFileName))
	{
		// can't replace file, so remove temporary file from disc and return error
		remove(strTmpName.c_str());
		return TEL_ERR_CANT_WRITE_FILE;
	}

	return TEL_ERR_OK;
}



