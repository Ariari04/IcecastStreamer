#include "Mp3Decoder.h"

#include <id3/tag.h>
#include <id3/utils.h>
#include <id3/misc_support.h>
#include <id3/readers.h>

#define NOANALYSIS
#include <mpg123.h>
#include "3rd/mpglib/interface.h"

#define SAMPLE_SIZE 4608
#define PIECE_SIZE 100

namespace Decoding
{

Mp3DecoderProducer::Mp3DecoderProducer(IProgressManager* progress)
    :m_stereo(true)
    ,m_frequency(44100)
    ,m_start(0)
    ,m_streamSize(0)
    ,m_bytesReaded(0)
    ,m_file(NULL)
    ,m_mp(NULL)
    ,m_progress(progress)
{}

Mp3DecoderProducer::~Mp3DecoderProducer()
{
    if (m_mp)
    {
        ExitMP3(m_mp);
        free(m_mp);
        m_mp = 0;
    }

    if (m_file)
    {
        fclose(m_file);
        m_file = 0;
    }
}

Conversion::SoundFormatInfo Mp3DecoderProducer::getSoundFormatInfo() const
{
	Conversion::SoundFormatInfo info;
    info.nSampleRate = m_frequency;
    info.nNumberOfChannels = m_stereo ? 2 : 1;
    info.nBitsPerSample = 16;
    return info;
}

bool Mp3DecoderProducer::readSamples(std::vector<byte>& samples)
{
    int ret = MP3_NEED_MORE;

    unsigned char readBuf[PIECE_SIZE];
    samples.resize(SAMPLE_SIZE);

    while (ret != MP3_OK)
    {
        // read a piece of data from file
        int count = fread(readBuf, 1, PIECE_SIZE, m_file);

        if (count <= 0)
        {
            // can't read from file
            return false;
        }

        m_bytesReaded += count;

        m_progress->reportProgress(m_bytesReaded, m_streamSize);

        int nDone = 0;
        // try to decode the piece
        ret = decodeMP3(m_mp, readBuf, count, (char*)&samples.front(), samples.size(), &nDone);

        if (ret == MP3_OK)
        {
            samples.resize(nDone);
        }
    }

    return true;
}

void Mp3DecoderProducer::open(const char* fileName)
{
    m_file = fopen(fileName, "rb");

    if (!m_file)
    {
        // FIXME: report error here
        return;
    }

    m_mp = (MPSTR*)malloc(0x10000);

    InitMP3(m_mp);
    readFileInfo(fileName);

    fseek(m_file, m_start, SEEK_SET);
}

void Mp3DecoderProducer::readFileInfo(const char* fileName)
{
    const Mp3_Headerinfo* header;
    ID3_Tag myTag;

    myTag.Link(fileName, ID3TT_ALL);

    if ((header = myTag.GetMp3HeaderInfo()) == NULL)
    {
        // FIXME: report error here
        return;
    }

    m_stereo = header->channelmode != MP3CHANNELMODE_SINGLE_CHANNEL;
    m_frequency = header->frequency;
    m_start = myTag.GetPrependedBytes();
    m_streamSize = myTag.GetFileSize() - myTag.GetPrependedBytes() - myTag.GetAppendedBytes();
}

}
