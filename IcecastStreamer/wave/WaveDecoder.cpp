#include "WaveDecoder.h"

#include <fstream>

#include "lame.h"

#include <iostream>
#include <vector>
#include <cstring>


#include <vorbis/vorbisenc.h>

namespace Decoding
{

	void WaveToMp3Decoder::close()
	{
		f.close();
		//fclose(file);
		//file = NULL;
	}

	WaveToMp3Decoder::~WaveToMp3Decoder()
	{
		f.close();
	}

	int WaveToMp3Decoder::open(const char* fileName)
	{
		f.close();

		//file = fopen(fileName, "rb");
		f.open(fileName, std::ios::binary);

		if (!f)
		{
			return 0;
		}

		size_t nBytesRead = 0;
		size_t nBytesToRead = 0;
		int nBytesToSeek = 0;

		//nBytesRead = fread(&(Header.m_cIFFHeader), 1, sizeof(Header.m_cIFFHeader), file);

		f.read(reinterpret_cast<char*>(&(Header.m_cIFFHeader)), sizeof(Header.m_cIFFHeader));

		if (!f)
		{
			Header.m_cIFFHeader = WaveFileChunks::IFFHeader();
			close();
			return 0;
		}

		if (!Header.m_cIFFHeader.isOk())
		{
			Header.m_cIFFHeader = WaveFileChunks::IFFHeader();
			close();
			return false;
		}


		WaveFileChunks::ChunkHeader tmpHeader;

		//while (!feof(file))
		while (!f.eof())
		{
			/*
			nBytesRead = fread(&(tmpHeader), 1, \
				sizeof(WaveFileChunks::ChunkHeader), file);

			if (nBytesRead != sizeof(WaveFileChunks::ChunkHeader))
			{
				close();
				return false;
			}*/

			f.read(reinterpret_cast<char*>(&(tmpHeader)), sizeof(WaveFileChunks::ChunkHeader));

			if (!f)
			{
				break;
			}


			if (0 == memcmp(tmpHeader.m_id, Header.m_cFormatChunk.m_id, 4))
			{
				//Format header found;
				break;
			}


			/*
			if (0 != fseek(file, tmpHeader.m_nChunkSize, SEEK_CUR))
			{
				close();
				return false;
			}*/


			f.seekg(tmpHeader.m_nChunkSize, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}


		}

		//if (feof(file))
		if (f.eof())
		{
			close();
			return false;
		}

		if (tmpHeader.m_nChunkSize < 16)
		{
			close();
			return false;
		}


		nBytesToSeek = tmpHeader.m_nChunkSize - (sizeof(Header.m_cFormatChunk) - 8);
		nBytesToRead =
			(nBytesToSeek > 0)
			?
			sizeof(Header.m_cFormatChunk) - 8
			:
			tmpHeader.m_nChunkSize;

		/*
		nBytesRead = fread(&(Header.m_cFormatChunk.m_nFormatTag)
			, 1
			, nBytesToRead
			, file);*/

		f.read(reinterpret_cast<char*>(&(Header.m_cFormatChunk.m_nFormatTag)), nBytesToRead);

		if (!f)
		{
			close();
			return false;
		}


		if (nBytesToSeek > 0)
		{
			/*
			if (0 != fseek(file, nBytesToSeek, SEEK_CUR))
			{
				close();
				return false;
			}*/


			f.seekg(nBytesToSeek, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}



		}
		/*
		if (nBytesRead != nBytesToRead)
		{
			close();
			return false;
		}*/

		// Chunk size is copied here.
		Header.m_cFormatChunk << tmpHeader;
		Header.m_cFormatChunk.m_nChunkSize = 18;

		if (Header.m_cFormatChunk.isOk() == false)
		{
			close();
			return false;
		}

		//while (!feof(file))
		while (!f.eof())
		{
			/*
			nBytesRead = fread(&(tmpHeader), 1, \
				sizeof(WaveFileChunks::ChunkHeader), file);

			if (nBytesRead != sizeof(WaveFileChunks::ChunkHeader))
			{
				close();
				return false;
			}*/

			f.read(reinterpret_cast<char*>(&tmpHeader), sizeof(WaveFileChunks::ChunkHeader));

			if (!f)
			{
				close();
				return false;
			}

			if (0 == memcmp(tmpHeader.m_id, Header.m_cDataChunk.m_id, 4))
			{
				//Data header found;
				break;
			}

			/*
			if (0 != fseek(file, tmpHeader.m_nChunkSize, SEEK_CUR))
			{
				close();
				return false;
			}*/

			f.seekg(tmpHeader.m_nChunkSize, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}
		}

		Header.m_cDataChunk << tmpHeader;


		//if (feof(file) && (0 != Header.m_cDataChunk.m_id))
		if (f.eof() && (0 != Header.m_cDataChunk.m_id))
		{
			close();
			return false;
		}


		std::cout << "f.tellg() after open = " << f.tellg() << std::endl;

		openMp3Output();

		return 1;
	}


	int WaveToMp3Decoder::openMp3Output()
	{

		
		lame = lame_init();
		lame_set_in_samplerate(lame, this->Header.m_cFormatChunk.m_nSamplesPerSec);
		lame_set_VBR(lame, vbr_default);
		lame_init_params(lame);

		return 1;
	}

	int WaveToMp3Decoder::readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration)
	{
		if (f.eof())
		{
			if (!flush_sent)
			{
				flush_sent = true;
				return lame_encode_flush(lame, reinterpret_cast<unsigned char*>(Buffer), Count);
			}
			else
			{
				close();
				return 0;
			}
			
		}

		auto readCount = (this->Header.m_cFormatChunk.m_nChannels * this->Header.m_cFormatChunk.m_nSamplesPerSec * this->Header.m_cFormatChunk.m_nBitsPerSample * duration.count() / 1000) / 8;
		
		std::vector<short> buf;
		buf.resize(readCount / sizeof(short));

		//std::cout << "readCount interleaved: " << readCount << std::endl;

		std::cout << "f.tellg() before = " << f.tellg() << std::endl;

		f.read(reinterpret_cast<char*>(&buf[0]), readCount);

		std::cout << "f.tellg() after = " << f.tellg() << std::endl;

		//int write = lame_encode_buffer_interleaved(lame, &buf[0], this->Header.m_cFormatChunk.m_nSamplesPerSec, reinterpret_cast<unsigned char*>(Buffer), Count);

		int write = lame_encode_buffer_interleaved(lame, &buf[0], buf.size()/ this->Header.m_cFormatChunk.m_nChannels, reinterpret_cast<unsigned char*>(Buffer), Count);

		return write;
		
	}


	//---------------------------------------------


	void WaveToOggDecoder::close()
	{
		f.close();
	}

	WaveToOggDecoder::~WaveToOggDecoder()
	{
		f.close();


		ogg_stream_clear(&os);
		vorbis_block_clear(&vb);
		vorbis_dsp_clear(&vd);
		vorbis_comment_clear(&vc);
		vorbis_info_clear(&vi);
	}

	int WaveToOggDecoder::open(const char* fileName)
	{
		f.close();

		//file = fopen(fileName, "rb");
		f.open(fileName, std::ios::binary);

		if (!f)
		{
			return 0;
		}

		size_t nBytesRead = 0;
		size_t nBytesToRead = 0;
		int nBytesToSeek = 0;

		//nBytesRead = fread(&(Header.m_cIFFHeader), 1, sizeof(Header.m_cIFFHeader), file);

		f.read(reinterpret_cast<char*>(&(Header.m_cIFFHeader)), sizeof(Header.m_cIFFHeader));

		if (!f)
		{
			Header.m_cIFFHeader = WaveFileChunks::IFFHeader();
			close();
			return 0;
		}

		if (!Header.m_cIFFHeader.isOk())
		{
			Header.m_cIFFHeader = WaveFileChunks::IFFHeader();
			close();
			return false;
		}


		WaveFileChunks::ChunkHeader tmpHeader;

		//while (!feof(file))
		while (!f.eof())
		{
			/*
			nBytesRead = fread(&(tmpHeader), 1, \
				sizeof(WaveFileChunks::ChunkHeader), file);

			if (nBytesRead != sizeof(WaveFileChunks::ChunkHeader))
			{
				close();
				return false;
			}*/

			f.read(reinterpret_cast<char*>(&(tmpHeader)), sizeof(WaveFileChunks::ChunkHeader));

			if (!f)
			{
				break;
			}


			if (0 == memcmp(tmpHeader.m_id, Header.m_cFormatChunk.m_id, 4))
			{
				//Format header found;
				break;
			}


			/*
			if (0 != fseek(file, tmpHeader.m_nChunkSize, SEEK_CUR))
			{
				close();
				return false;
			}*/


			f.seekg(tmpHeader.m_nChunkSize, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}


		}

		//if (feof(file))
		if (f.eof())
		{
			close();
			return false;
		}

		if (tmpHeader.m_nChunkSize < 16)
		{
			close();
			return false;
		}


		nBytesToSeek = tmpHeader.m_nChunkSize - (sizeof(Header.m_cFormatChunk) - 8);
		nBytesToRead =
			(nBytesToSeek > 0)
			?
			sizeof(Header.m_cFormatChunk) - 8
			:
			tmpHeader.m_nChunkSize;

		/*
		nBytesRead = fread(&(Header.m_cFormatChunk.m_nFormatTag)
			, 1
			, nBytesToRead
			, file);*/

		f.read(reinterpret_cast<char*>(&(Header.m_cFormatChunk.m_nFormatTag)), nBytesToRead);

		if (!f)
		{
			close();
			return false;
		}


		if (nBytesToSeek > 0)
		{
			/*
			if (0 != fseek(file, nBytesToSeek, SEEK_CUR))
			{
				close();
				return false;
			}*/


			f.seekg(nBytesToSeek, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}



		}
		/*
		if (nBytesRead != nBytesToRead)
		{
			close();
			return false;
		}*/

		// Chunk size is copied here.
		Header.m_cFormatChunk << tmpHeader;
		Header.m_cFormatChunk.m_nChunkSize = 18;

		if (Header.m_cFormatChunk.isOk() == false)
		{
			close();
			return false;
		}

		//while (!feof(file))
		while (!f.eof())
		{
			/*
			nBytesRead = fread(&(tmpHeader), 1, \
				sizeof(WaveFileChunks::ChunkHeader), file);

			if (nBytesRead != sizeof(WaveFileChunks::ChunkHeader))
			{
				close();
				return false;
			}*/

			f.read(reinterpret_cast<char*>(&tmpHeader), sizeof(WaveFileChunks::ChunkHeader));

			if (!f)
			{
				close();
				return false;
			}

			if (0 == memcmp(tmpHeader.m_id, Header.m_cDataChunk.m_id, 4))
			{
				//Data header found;
				break;
			}

			/*
			if (0 != fseek(file, tmpHeader.m_nChunkSize, SEEK_CUR))
			{
				close();
				return false;
			}*/

			f.seekg(tmpHeader.m_nChunkSize, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}
		}

		Header.m_cDataChunk << tmpHeader;


		//if (feof(file) && (0 != Header.m_cDataChunk.m_id))
		if (f.eof() && (0 != Header.m_cDataChunk.m_id))
		{
			close();
			return false;
		}


		std::cout << "f.tellg() after open = " << f.tellg() << std::endl;

		//openMp3Output();
		openOggOutput();

		return 1;
	}


	int WaveToOggDecoder::openOggOutput()
	{

		fout = std::ofstream("oggtest.ogg", std::ios::out | std::ios::binary);
		vorbis_info_init(&vi);

		int ret;

		int write = 0;

		ret = vorbis_encode_init_vbr(&vi, 2, 44100, 0.1);

		vorbis_comment_init(&vc);
		vorbis_comment_add_tag(&vc, "ENCODER", "icecast streamer");

		vorbis_analysis_init(&vd, &vi);
		vorbis_block_init(&vd, &vb);

		/* set up our packet->stream encoder */
		/* pick a random serial number; that way we can more likely build
		   chained streams just by concatenation */
		srand(time(NULL));
		ogg_stream_init(&os, rand());


		{
			ogg_packet header;
			ogg_packet header_comm;
			ogg_packet header_code;

			vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
			ogg_stream_packetin(&os, &header); /* automatically placed in its own
												 page */
			ogg_stream_packetin(&os, &header_comm);
			ogg_stream_packetin(&os, &header_code);

			/* This ensures the actual
			 * audio data will start on a new page, as per spec
			 */
			while (true) {
				int result = ogg_stream_flush(&os, &og);
				if (result == 0) break;
				this->dataToSend.resize(this->dataToSend.size() + og.header_len + og.body_len);

				std::copy(og.header, og.header + og.header_len, &dataToSend[write]);
				write += og.header_len;

				std::copy(og.body, og.body + og.body_len, &dataToSend[write]);
				write += og.body_len;
			}

		}

		//f.close();

		//f.open("oggtest.ogg", std::ios::binary);


		return 1;
	}

	int WaveToOggDecoder::readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration)
	{
		if (f.eof())
		{
			close();
			fout.close();
			return 0;
		}

		int write = 0;

		if (dataToSend.size() != 0)
		{
			write = dataToSend.size();
			std::copy(dataToSend.begin(), dataToSend.end(), Buffer);

			fout.write(&dataToSend[0], dataToSend.size());
			fout.flush();

			dataToSend.clear();
		}

		int readCount = (this->Header.m_cFormatChunk.m_nChannels * this->Header.m_cFormatChunk.m_nSamplesPerSec * this->Header.m_cFormatChunk.m_nBitsPerSample * duration.count() / 1000) / 8;

		int eos = 0;

		std::vector<char> buf_char;
		buf_char.resize(readCount * 4 + 44);

		std::cout << "f.tellg() before = " << f.tellg() << std::endl;

		f.read(&buf_char[0], readCount * 4);

		auto bytes = f.gcount();

		std::cout << "f.tellg() after = " << f.tellg() << std::endl;



		//while (!eos)
		{
			if (bytes == 0)
			{
				vorbis_analysis_wrote(&vd, 0);
			}
			else
			{
				float** buffer = vorbis_analysis_buffer(&vd, readCount);

				// uninterleave samples 
				for (int i = 0; i < bytes / 4; i++) {

					buffer[0][i] = ((buf_char[i * 4 + 1] << 8) |
						(0x00ff & (int)buf_char[i * 4])) / 32768.f;
					buffer[1][i] = ((buf_char[i * 4 + 3] << 8) |
						(0x00ff & (int)buf_char[i * 4 + 2])) / 32768.f;

					//buffer[0][i] = 0;
					//buffer[1][i] = 0;
				}

				// tell the library how much we actually submitted 
				vorbis_analysis_wrote(&vd, bytes / 4);
			}

			while (vorbis_analysis_blockout(&vd, &vb) == 1) {

				// analysis, assume we want to use bitrate management 
				vorbis_analysis(&vb, NULL);
				vorbis_bitrate_addblock(&vb);

				while (vorbis_bitrate_flushpacket(&vd, &op)) {

					// weld the packet into the bitstream 
					ogg_stream_packetin(&os, &op);

					// write out pages (if any) 
					while (!eos) {
						int result = ogg_stream_pageout(&os, &og);
						if (result == 0) break;

						std::copy(og.header, og.header + og.header_len, Buffer + write);

						write += og.header_len;

						fout.write(reinterpret_cast<char*>(og.header), og.header_len);


						std::copy(og.body, og.body + og.body_len, Buffer + write);

						fout.write(reinterpret_cast<char*>(og.body), og.body_len);

						fout.flush();

						write += og.body_len;

						// this could be set above, but for illustrative purposes, I do
						//   it here (to show that vorbis does know where the stream ends) 

						if (ogg_page_eos(&og))
						{
							eos = 1;
						}
					}
				}
			}



		}


		//int write = lame_encode_buffer_interleaved(lame, &buf[0], buf.size() / this->Header.m_cFormatChunk.m_nChannels, reinterpret_cast<unsigned char*>(Buffer), Count);

		return write;
		


		/*
		if (f.eof())
		{
			if (!flush_sent)
			{
				flush_sent = true;
				return lame_encode_flush(lame, reinterpret_cast<unsigned char*>(Buffer), Count);
			}
			else
			{
				close();
				return 0;
			}

		}

		auto readCount = (this->Header.m_cFormatChunk.m_nChannels * this->Header.m_cFormatChunk.m_nSamplesPerSec * this->Header.m_cFormatChunk.m_nBitsPerSample * duration.count() / 1000) / 8;

		std::vector<short> buf;
		buf.resize(readCount / sizeof(short));

		//std::cout << "readCount interleaved: " << readCount << std::endl;

		std::cout << "f.tellg() before = " << f.tellg() << std::endl;

		f.read(reinterpret_cast<char*>(&buf[0]), readCount);

		std::cout << "f.tellg() after = " << f.tellg() << std::endl;

		//int write = lame_encode_buffer_interleaved(lame, &buf[0], this->Header.m_cFormatChunk.m_nSamplesPerSec, reinterpret_cast<unsigned char*>(Buffer), Count);

		int write = lame_encode_buffer_interleaved(lame, &buf[0], buf.size() / this->Header.m_cFormatChunk.m_nChannels, reinterpret_cast<unsigned char*>(Buffer), Count);
		
		return write;*/

	}

	//--------------------------


	//---------------------------------------------


	void OggToOggDecoder::close()
	{
		f.close();
	}

	OggToOggDecoder::~OggToOggDecoder()
	{
		f.close();


		ogg_stream_clear(&os);
		vorbis_block_clear(&vb);
		vorbis_dsp_clear(&vd);
		vorbis_comment_clear(&vc);
		vorbis_info_clear(&vi);
	}

	int OggToOggDecoder::open(const char* fileName)
	{
		f.close();

		//file = fopen(fileName, "rb");
		f.open(fileName, std::ios::binary);

		if (!f)
		{
			return 0;
		}

		size_t nBytesRead = 0;
		size_t nBytesToRead = 0;
		int nBytesToSeek = 0;

		//nBytesRead = fread(&(Header.m_cIFFHeader), 1, sizeof(Header.m_cIFFHeader), file);

		f.read(reinterpret_cast<char*>(&(Header.m_cIFFHeader)), sizeof(Header.m_cIFFHeader));

		if (!f)
		{
			Header.m_cIFFHeader = WaveFileChunks::IFFHeader();
			close();
			return 0;
		}

		if (!Header.m_cIFFHeader.isOk())
		{
			Header.m_cIFFHeader = WaveFileChunks::IFFHeader();
			close();
			return false;
		}


		WaveFileChunks::ChunkHeader tmpHeader;

		//while (!feof(file))
		while (!f.eof())
		{
			/*
			nBytesRead = fread(&(tmpHeader), 1, \
				sizeof(WaveFileChunks::ChunkHeader), file);

			if (nBytesRead != sizeof(WaveFileChunks::ChunkHeader))
			{
				close();
				return false;
			}*/

			f.read(reinterpret_cast<char*>(&(tmpHeader)), sizeof(WaveFileChunks::ChunkHeader));

			if (!f)
			{
				break;
			}


			if (0 == memcmp(tmpHeader.m_id, Header.m_cFormatChunk.m_id, 4))
			{
				//Format header found;
				break;
			}


			/*
			if (0 != fseek(file, tmpHeader.m_nChunkSize, SEEK_CUR))
			{
				close();
				return false;
			}*/


			f.seekg(tmpHeader.m_nChunkSize, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}


		}

		//if (feof(file))
		if (f.eof())
		{
			close();
			return false;
		}

		if (tmpHeader.m_nChunkSize < 16)
		{
			close();
			return false;
		}


		nBytesToSeek = tmpHeader.m_nChunkSize - (sizeof(Header.m_cFormatChunk) - 8);
		nBytesToRead =
			(nBytesToSeek > 0)
			?
			sizeof(Header.m_cFormatChunk) - 8
			:
			tmpHeader.m_nChunkSize;

		/*
		nBytesRead = fread(&(Header.m_cFormatChunk.m_nFormatTag)
			, 1
			, nBytesToRead
			, file);*/

		f.read(reinterpret_cast<char*>(&(Header.m_cFormatChunk.m_nFormatTag)), nBytesToRead);

		if (!f)
		{
			close();
			return false;
		}


		if (nBytesToSeek > 0)
		{
			/*
			if (0 != fseek(file, nBytesToSeek, SEEK_CUR))
			{
				close();
				return false;
			}*/


			f.seekg(nBytesToSeek, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}



		}
		/*
		if (nBytesRead != nBytesToRead)
		{
			close();
			return false;
		}*/

		// Chunk size is copied here.
		Header.m_cFormatChunk << tmpHeader;
		Header.m_cFormatChunk.m_nChunkSize = 18;

		if (Header.m_cFormatChunk.isOk() == false)
		{
			close();
			return false;
		}

		//while (!feof(file))
		while (!f.eof())
		{
			/*
			nBytesRead = fread(&(tmpHeader), 1, \
				sizeof(WaveFileChunks::ChunkHeader), file);

			if (nBytesRead != sizeof(WaveFileChunks::ChunkHeader))
			{
				close();
				return false;
			}*/

			f.read(reinterpret_cast<char*>(&tmpHeader), sizeof(WaveFileChunks::ChunkHeader));

			if (!f)
			{
				close();
				return false;
			}

			if (0 == memcmp(tmpHeader.m_id, Header.m_cDataChunk.m_id, 4))
			{
				//Data header found;
				break;
			}

			/*
			if (0 != fseek(file, tmpHeader.m_nChunkSize, SEEK_CUR))
			{
				close();
				return false;
			}*/

			f.seekg(tmpHeader.m_nChunkSize, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}
		}

		Header.m_cDataChunk << tmpHeader;


		//if (feof(file) && (0 != Header.m_cDataChunk.m_id))
		if (f.eof() && (0 != Header.m_cDataChunk.m_id))
		{
			close();
			return false;
		}


		std::cout << "f.tellg() after open = " << f.tellg() << std::endl;

		openOggOutput();

		return 1;
	}


	int OggToOggDecoder::openOggOutput()
	{

		//fout = std::ofstream("oggtest.ogg", std::ios::out | std::ios::binary);
		vorbis_info_init(&vi);

		int ret;

		ret = vorbis_encode_init_vbr(&vi, 2, 44100, 0.1);

		vorbis_comment_init(&vc);
		vorbis_comment_add_tag(&vc, "ENCODER", "icecast streamer");

		vorbis_analysis_init(&vd, &vi);
		vorbis_block_init(&vd, &vb);

		/* set up our packet->stream encoder */
		/* pick a random serial number; that way we can more likely build
		   chained streams just by concatenation */
		srand(time(NULL));
		ogg_stream_init(&os, rand());


		{
			ogg_packet header;
			ogg_packet header_comm;
			ogg_packet header_code;

			vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
			ogg_stream_packetin(&os, &header); /* automatically placed in its own
												 page */
			ogg_stream_packetin(&os, &header_comm);
			ogg_stream_packetin(&os, &header_code);

			/* This ensures the actual
			 * audio data will start on a new page, as per spec
			 */
			while (true) {
				int result = ogg_stream_flush(&os, &og);
				if (result == 0) break;
				this->dataToSend.resize(og.header_len + og.body_len);
				std::copy(og.header, og.header + og.header_len, &dataToSend[0]);
				std::copy(og.body, og.body + og.body_len, &dataToSend[og.header_len]);

				//fout.write(&dataToSend[0], dataToSend.size());
			}

		}

		f.close();

		f.open("oggtest.ogg", std::ios::binary);


		return 1;
	}

	int OggToOggDecoder::readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration)
	{
		if (f.eof())
		{
			close();
			//fout.close();

			return 0;
		}


		auto readCount = 1000;

		std::vector<char> buf_char;
		buf_char.resize(readCount);

		std::cout << "f.tellg() before = " << f.tellg() << std::endl;

		f.read(&buf_char[0], readCount);

		std::cout << "f.tellg() after = " << f.tellg() << std::endl;

		std::copy(buf_char.begin(), buf_char.end(), Buffer);

		return readCount;

	}
}
