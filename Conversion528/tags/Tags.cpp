#include "../stdafx.h"
#include "Tags.h"
#include <memory>
#include <sys/stat.h>

#ifdef WIN32
#include "TagsUsingWMF.h"
#endif

#include "TagsUsingMP4v2.h"
#include "ID3Tags.h"
#include "../PathOperations.h"
#include <cstring>

const char* const GENRE_NAMES[GENRE_COUNT] =
{
    /*
     * NOTE: The spelling of these genre names is identical to those found in
     * Winamp and mp3info.
     */
    "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge",
    "Hip-Hop", "Jazz", "Meta", "New Age", "Oldies", "Other", "Pop", "R&B",
    "Rap", "Reggae", "Rock", "Techno", "Industria", "Alternative", "Ska",
    "Death Meta", "Pranks", "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop",
    "Voca", "Jazz+Funk", "Fusion", "Trance", "Classica", "Instrumenta",
    "Acid", "House", "Game", "Sound Clip", "Gospe", "Noise", "Alt. Rock",
    "Bass", "Sou", "Punk", "Space", "Meditative", "Instrumental Pop",
    "Instrumental Rock", "Ethnic", "Gothic", "Darkwave", "Techno-Industria",
    "Electronic", "Pop-Folk", "Eurodance", "Dream", "Southern Rock", "Comedy",
    "Cult", "Gangsta Rap", "Top 40", "Christian Rap", "Pop/Funk", "Jungle",
    "Native American", "Cabaret", "New Wave", "Psychedelic", "Rave",
    "Showtunes", "Trailer", "Lo-Fi", "Triba", "Acid Punk", "Acid Jazz",
    "Polka", "Retro", "Musica", "Rock & Rol", "Hard Rock", "Folk",
    "Folk/Rock", "National Folk", "Swing", "Fast-Fusion", "Bebob", "Latin",
    "Reviva", "Celtic", "Bluegrass", "Avantgarde", "Gothic Rock",
    "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock",
    "Big Band", "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech",
    "Chanson", "Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass",
    "Primus", "Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba",
    "Folklore", "Ballad", "Power Ballad", "Rhythmic Sou", "Freestyle", "Duet",
    "Punk Rock", "Drum Solo", "A Cappella", "Euro-House", "Dance Hal",
    "Goa", "Drum & Bass", "Club-House", "Hardcore", "Terror", "Indie",
    "BritPop", "Negerpunk", "Polsk Punk", "Beat", "Christian Gangsta Rap",
    "Heavy Meta", "Black Meta", "Crossover", "Contemporary Christian",
    "Christian Rock", "Merengue", "Salsa", "Thrash Meta", "Anime", "JPop",
    "Synthpop"
};


namespace
{
	inline bool CheckFileExists(const char* fullPath)
	{
		struct stat fileStat;

		if (0 != stat(fullPath, &fileStat))
		{
			return TEL_ERR_CANT_OPEN_FILE;
		}

		return TEL_ERR_OK;
	}

}// namespace


TelError ReadTagsFromFile(const char* strFileName, KTags& rTags)
{
	if(CheckFileExists(strFileName) == TEL_ERR_CANT_OPEN_FILE)
	{
		return TEL_ERR_CANT_OPEN_FILE;
	}

	rTags.strFileName = strFileName;

	std::string strExt = PathOperations::GetExtension(strFileName);

	if (strcasecmp(strExt.c_str(), "aac") == 0 ||
		strcasecmp(strExt.c_str(), "mp3") == 0)
	{
		return ReadTagsFromFileID3(strFileName, rTags);
	}
	else if (strcasecmp(strExt.c_str(), "m4a") == 0 ||
			 strcasecmp(strExt.c_str(), "mp4") == 0)
	{
		return ReadTagsFromFileMP4(strFileName, rTags);
	}
	else
	{
		return TEL_ERR_UNKNOWN_FORMAT;
	}
}

TelError WriteTagsToFile(const char* strFileName,const KTags& rTags)
{
	if(CheckFileExists(strFileName))
	{
		return TEL_ERR_CANT_OPEN_FILE;
	}

	std::string strExt = PathOperations::GetExtension(strFileName);

	if (strcasecmp(strExt.c_str(), "aac") == 0 ||
		strcasecmp(strExt.c_str(), "mp3") == 0)
	{
		return WriteTagsToFileID3(strFileName, rTags);
	}
	else if(strcasecmp(strExt.c_str(), "m4a") == 0 ||
			strcasecmp(strExt.c_str(), "mp4") == 0)
	{
		return WriteTagsToFileMP4(strFileName, rTags);
	}
	else
	{
		return TEL_ERR_UNKNOWN_FORMAT;
	}
}

TelError KTags::WriteTags(const std::string& strFile) const
{
	const std::string& strFilePath = strFile.empty() ? strFileName : strFile;
	return WriteTagsToFile(strFilePath.c_str(), *this);
}

TelError KTags::ReadTags(const std::string& strFile)
{
	const std::string& strFilePath = strFile.empty() ? strFileName : strFile;
	return ReadTagsFromFile(strFilePath.c_str(), *this);
}

KTags::KTags()
{
	ClearContent();
	strFileName = "";
}

void KTags::ClearContent()
{
	m_cTagDictionary.clear();
	m_cMP4Tags.clear();
	SetID3VXFlags();
}

void KTags::SetTag(int nTagID, const KString& strTagValue)
{
	KDictionary::iterator it = m_cTagDictionary.find(nTagID);

	if (it != m_cTagDictionary.end())
	{
		if (strTagValue.GetLength() == 0)
		{
			m_cTagDictionary.erase(it);
		}
		else
		{
			it->second = strTagValue;
		}
	}
	else
	{
		if (strTagValue.GetLength() != 0)
		{
			m_cTagDictionary.insert(std::make_pair(nTagID, strTagValue));
		}
	}
}

KString KTags::GetTag(int nKey) const
{
	KDictionary::const_iterator it = m_cTagDictionary.find(nKey);
	if (it != m_cTagDictionary.end())
		return (*it).second;
	else
		return L"";
}

void KTags::SetMP4Tag(int nTagID, const KString& strTagValue)
{
	KDictionary::iterator it = m_cMP4Tags.find(nTagID);

	if (it != m_cMP4Tags.end())
	{
		if (strTagValue.GetLength() == 0)
			m_cMP4Tags.erase(it);
		else
			it->second = strTagValue;
	}
	else
	{
		if (strTagValue.GetLength() != 0)
			m_cMP4Tags.insert(std::make_pair(nTagID, strTagValue));
	}
}

KString KTags::GetMP4Tag(int nKey) const
{
	KDictionary::const_iterator it = m_cMP4Tags.find(nKey);
	if (it != m_cMP4Tags.end())
		return (*it).second;
	else
		return L"";
}

KTags::const_iterator KTags::GetBegin() const
{
	return m_cTagDictionary.begin();
}

KTags::const_iterator KTags::GetEnd() const
{
	return m_cTagDictionary.end();
}

KTags::KTags(const KTags& rhs)
{
	*this = rhs;
}

KTags& KTags::operator =(const KTags &rhs)
{
	CopyContent(rhs);
	strFileName = rhs.strFileName;
	return *this;
}

void KTags::CopyContent(const KTags& rhs)
{
	// Note strFileName unmodified.
	// Only content is copied.
	// This is the difference with operator=();
	m_cTagDictionary = rhs.m_cTagDictionary;
	m_cMP4Tags = rhs.m_cMP4Tags;
	bHasID3v1Tags = rhs.bHasID3v1Tags;
	bHasID3v2Tags = rhs.bHasID3v2Tags;
}

void KTags::SetID3VXFlags()
{
	bHasID3v1Tags = bHasID3v2Tags = false;

	for(int i = TAG_ID3V1_START; i < TAG_ID3V1_FIN; ++i)
	{
		if (GetTag(i).GetLength())
		{
			bHasID3v1Tags = true;
			break;
		}
	}

	for(int i = TAG_ID3V2_START; i < TAG_ID3V2_FIN; ++i)
	{
		if (GetTag(i).GetLength())
		{
			bHasID3v2Tags = true;
			break;
		}
	}
}

void KTags::CopyID3v1ToID3v2()
{
	for(int i = TAG_ID3V1_START; i<TAG_ID3V1_FIN; ++i)
	{
		SetTag(TAG_ID3V2_START + i - TAG_ID3V1_START, GetTag(i));
	}
	this->SetID3VXFlags();
}

void KTags::CopyID3v2ToID3v1()
{
	for(int i = TAG_ID3V1_START; i<TAG_ID3V1_FIN; ++i)
	{
		SetTag(i, GetTag(TAG_ID3V2_START + i - TAG_ID3V1_START));
	}
	this->SetID3VXFlags();
}

KTags MakeTagsContentUnion(const KTags& cV1Tags, const KTags& cV2Tags, const KTags& cAWTags)
{
	KTags cRes(cV2Tags);
	for (int i = TAG_ID3V1_START; i < TAG_ID3V1_FIN; ++i)
	{
		cRes.SetTag(i, cV1Tags.GetTag(i));
	}

	cRes.SetID3VXFlags();
	cRes.m_cMP4Tags = cV2Tags.m_cMP4Tags;
	return cRes;
}

void KTags::DumpMP4TagsToID3Comments()
{

	KString strTmp ="";

	if(m_cMP4Tags[TAG_MP4_TOOL].GetLength())
	{
		strTmp += KString("\n") + KString(DESC_TAG_MP4_TOOL) + ": " + m_cMP4Tags[TAG_MP4_TOOL];
	}

	m_cTagDictionary[TAG_COMMENT_2] += strTmp;
	m_cTagDictionary[TAG_COMMENT] += strTmp;
}

void KTags::DumpOggTagsToID3Comments()
{
	KString strTmp = "";
	m_cTagDictionary[TAG_COMMENT_2] += strTmp;
	m_cTagDictionary[TAG_COMMENT] += strTmp;
}

KTags KTags::MakeFullID3Copy() const
{
	KTags res(*this);
	res.DumpMP4TagsToID3Comments();
	res.DumpOggTagsToID3Comments();
	return res;
}

void KTags::AddStringToID3V2Comments(const KString& str)
{
	KString strComments = GetTag(TAG_COMMENT_2);
	if(strComments.GetLength())
	{
		strComments += "\n";
	}
	SetTag(TAG_COMMENT_2, strComments);
}

KTags::FileAttributes::FileAttributes()
	:playTime(0)
{
}

const KTags::FileAttributes& KTags::getAttributes() const
{
	return m_attributes;
}

KTags::FileAttributes& KTags::getAttributes()
{
	return m_attributes;
}
