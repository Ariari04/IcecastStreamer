#pragma once

#include <cstdio>
#include <wave/WaveFile.h>
#include <AudioFile.h>
#include <fstream>

#include <vector>

#include <vorbis/vorbisenc.h>

namespace Decoding
{
	

	class WaveDecoder : public AudioDecoderInterface
	{
	public:
		std::ifstream f;

		WaveFileChunks::WaveFileHeader Header;

		~WaveDecoder();

		virtual int readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration, std::chrono::milliseconds& actualDurationRead) override;

		void close();
		
		virtual bool open(const char* fileName) override;
	};

	class WavToOggConverter
	{
	public:
		vorbis_info vi;
		vorbis_comment vc;
		vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
		vorbis_block     vb; /* local working space for packet->PCM decode */

		ogg_stream_state os; /* take physical pages, weld into a logical
						  stream of packets */

		ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
		ogg_packet       op; /* one raw packet of data for decode */


		std::vector<char> dataToSend;

		int eos = 0;

		void openOutput();

		int convertData(const char* inputBuffer, size_t inputCount, char* Buffer, size_t Count);

		int finishConvertData(char* Buffer, size_t Count);

	};

	class OggDecoder : public AudioDecoderInterface
	{
	public:
		std::ifstream f;

		ogg_sync_state   oy; /* sync and verify incoming physical bitstream */
		ogg_stream_state os; /* take physical pages, weld into a logical
								stream of packets */
		ogg_page         og; /* one Ogg bitstream page. Vorbis packets are inside */
		ogg_packet       op; /* one raw packet of data for decode */

		vorbis_info      vi; /* struct that stores all the static vorbis bitstream
								settings */
		vorbis_comment   vc; /* struct that stores all the bitstream user comments */
		vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
		vorbis_block     vb; /* local working space for packet->PCM decode */


		int eos = 0;

		std::vector<char> tempBuffer;

		~OggDecoder();

		virtual int readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration, std::chrono::milliseconds& actualDurationRead) override;

		void close();

		virtual bool open(const char* fileName) override;
	};


}