#ifndef _MY_WAVE_FILE_CONVERSION_ADAPTER_H_
#define _MY_WAVE_FILE_CONVERSION_ADAPTER_H_

#include "Conversion.h"
#include "WaveFile.h"

namespace WaveFile
{
	class WaveProducer : public Conversion::ISoundProducer
	{
	public:
		WaveProducer(WaveFile::WaveFileReader& reader);
		virtual Conversion::SoundFormatInfo getSoundFormatInfo() const;
		virtual uint32	getSound(byte* pBuffer, uint32 nBufferSize);
	private:
		Conversion::SoundFormatInfo m_sSoundFormatInfo;
		WaveFileReader& m_cReader;
	};

	class WaveConsumer : public Conversion::ISoundConsumer
	{
	public:
		WaveConsumer(WaveFile::WaveFileWriter& writer);
		~WaveConsumer();
		virtual bool setSoundFormatInfo(const Conversion::SoundFormatInfo& sInfo);
		virtual uint32 putSound(const byte* pBuffer, uint32 nBufferSize);
		virtual bool writeHeader();
	private:
		WaveFileWriter& m_cWriter;
		uint32          m_soundSize;
	};
}

#endif //_MY_WAVE_FILE_CONVERSION_ADAPTER_H_
