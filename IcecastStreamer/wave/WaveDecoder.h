#pragma once

#include <cstdio>
#include <wave/WaveFile.h>
#include <AudioFile.h>
#include <fstream>
#include "lame.h"

#include <vector>

#include <vorbis/vorbisenc.h>

namespace Decoding
{
	   
	class WaveToMp3Decoder : public AudioDecoder
	{
	public:
		
		std::ifstream f;

		WaveFileChunks::WaveFileHeader Header;

		lame_t lame = nullptr;

		bool flush_sent = false;

		~WaveToMp3Decoder();

		void close();

		int open(const char* fileName) override;

		int openMp3Output();

		int readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration) override;
	};


	class WaveToOggDecoder : public AudioDecoder
	{
	public:

		std::ifstream f;

		std::ofstream fout;

		WaveFileChunks::WaveFileHeader Header;

		//lame_t lame = nullptr;

		bool flush_sent = false;


		vorbis_info vi;
		vorbis_comment vc;
		vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
		vorbis_block     vb; /* local working space for packet->PCM decode */

		ogg_stream_state os; /* take physical pages, weld into a logical
						  stream of packets */

		ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
		ogg_packet       op; /* one raw packet of data for decode */

		std::vector<char> dataToSend;

		~WaveToOggDecoder();

		void close();

		int open(const char* fileName) override;

		int openOggOutput();

		int readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration) override;
	};


	class OggToOggDecoder : public AudioDecoder
	{
	public:

		std::ifstream f;

		std::ofstream fout;

		WaveFileChunks::WaveFileHeader Header;

		//lame_t lame = nullptr;

		bool flush_sent = false;


		vorbis_info vi;
		vorbis_comment vc;
		vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
		vorbis_block     vb; /* local working space for packet->PCM decode */

		ogg_stream_state os; /* take physical pages, weld into a logical
						  stream of packets */

		ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
		ogg_packet       op; /* one raw packet of data for decode */

		std::vector<char> dataToSend;

		~OggToOggDecoder();

		void close();

		int open(const char* fileName) override;

		int openOggOutput();

		int readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration) override;
	};


}