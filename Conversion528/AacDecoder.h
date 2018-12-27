//#pragma once
//
//#include "DecoderAdapter.h"
////#include <interface/interface.h>
//#include <vector>
//#include "Progress.h"
//
//namespace Decoding
//{
//
//class AacDecoderProducer : public BufferedProducerBase
//{
//public:
//    AacDecoderProducer(IProgressManager* progress);
//    ~AacDecoderProducer();
//
//	virtual Conversion::SoundFormatInfo getSoundFormatInfo() const;
//
//protected:
//    virtual bool readSamples(std::vector<byte>& samples);
//
//    void open(const char* fileName) override;
//	void open(const wchar_t* fileName) override;
//
//private:
//	void*           m_hFile;
//	//aacheaderInfo*  m_fileInfo;
//	char**          m_ppData;
//	std::vector<byte> m_remaining;
//	IProgressManager* m_progress;
//};
//
//}
