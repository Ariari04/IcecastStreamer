#pragma once

//#include "EncLibInterfaces.h"
#include "../KString.h"
#include <map>
#include <vector>
#include "../PathOperations.h"
//#include "../IntegerTypes.h"

typedef std::map<int, KString> KDictionary;
typedef int TelError;

enum TelTagField
{
	TAG_INVALID = 0,

	TAG_START = 1,

	TAG_ID3V1_START = 1,
	TAG_TITLE = 1,
	TAG_ARTIST = 2,
	TAG_ALBUM = 3,
	TAG_GENRE = 4,
	TAG_COMMENT = 5,
	TAG_TRACK = 6,
	TAG_YEAR = 7,
	TAG_ID3V1_FIN = 8,

	TAG_ID3V2_START	= 9,
	TAG_TITLE_2	= 9,
	TAG_ARTIST_2 = 10,
	TAG_ALBUM_2	= 11,
	TAG_GENRE_2	= 12,
	TAG_COMMENT_2 = 13,
	TAG_TRACK_2	= 14,
	TAG_YEAR_2	= 15,

	TAG_ID3V2_ADDITIONAL_TAGS_START = 16,
	TAG_PUBLISHER = 16,
	TAG_ORIGARTIST = 17,
	TAG_COPYRIGHT = 18,
	TAG_DISC = 19,
	TAG_URL	= 20,
	TAG_COMPOSER = 21,
	TAG_BPM	= 22,
	TAG_ENCODED_BY = 23,
	TAG_ALBUM_ARTIST = 24,
	TAG_RATING = 25,
	TAG_ID3V2_FIN = 26,

	TAG_FIN	= 26
};

#define		TEL_ERR_OK							0	//No errors
#define		TEL_ERR_HANDLE						1	//TelBuffer or TelEncoder Handle is incorrect
#define		TEL_ERR_PARAM						2
#define		TEL_ERR_MOREMEMMORY					3
#define		TEL_ERR_INTERNAL					4
#define		TEL_ERR_CANT_OPEN_FILE				5
#define		TEL_ERR_UNKNOWN_FORMAT				6
#define		TEL_ERR_UNIMPLEMENTED				7
#define		TEL_ERR_UNKNOWN_PLAYLIST_FORMAT		8
#define		TEL_ERR_BAD_PLAYLIST				9
#define		TEL_ERR_BAD_STAGS_STRUCTURE			10
#define		TEL_ERR_BAD_PROJECT_DIR				11
#define		TEL_ERR_CANT_WRITE_FILE				12
#define		TEL_ERR_TOO_BIG_PLAYLIST			13
#define		TEL_ERR_TOO_BIG_PROJECT				14
#define		TEL_ERR_FILE_ALLREADY_EXISTS		15
#define		TEL_ERR_WRITE_ARTWORK				16
#define		TEL_ERR_ENCODING					17	//Error during encoding. See log file for more information

const int GENRE_COUNT = 148;
extern const char* const GENRE_NAMES[GENRE_COUNT];

#define TAG_MP4_TOOL				1

#define DESC_TAG_MP4_COLLECTION			L"Compilation"
#define DESC_TAG_MP4_TOOL				L"Tool"

class KTags
{
public:
	KDictionary		m_cMP4Tags;

	// ID3vX
	bool bHasID3v2Tags;
	bool bHasID3v1Tags;

	struct FileAttributes
	{
		FileAttributes();

		int playTime;
	};

	// Please, note that strFileName is not a part of tag's content.
	// It's just a useful info for WriteTags and ReadTags
	std::string strFileName;

public:

	typedef KDictionary::const_iterator const_iterator;

	KTags();
	KTags(const KTags& rhs);
	KTags& operator =(const KTags&rhs);
 	void CopyContent(const KTags& rhs);

	void ClearContent();

	KString GetTag(int nTagID) const;
	void SetTag(int nTagID, const KString& strTagValue);

	KString GetMP4Tag(int nTagID) const;
	void SetMP4Tag(int nTagID, const KString& strTagValue);

	const_iterator GetBegin() const;
	const_iterator GetEnd() const;

	void SetID3VXFlags();
	void CopyID3v1ToID3v2();
	void CopyID3v2ToID3v1();

	//TelError WriteTags(const std::string& strFile = "") const;
	TelError ReadTags(const std::string& strFile = "");

	void DumpOggTagsToID3Comments();
	void DumpMP4TagsToID3Comments();

	KTags MakeFullID3Copy() const;

	void AddStringToID3V2Comments(const KString& str);

	FileAttributes& getAttributes();
	const FileAttributes& getAttributes() const;

protected:
	KDictionary		m_cTagDictionary;
	FileAttributes  m_attributes;

};


TelError ReadTagsFromFile(const char* strFileName, KTags& rTags);

//TelError WriteTagsToFile(const char* strFileName,const KTags& rTags);

KTags MakeTagsContentUnion(const KTags& cV1Tags, const KTags& cV2Tags, const KTags& cAWTags);

