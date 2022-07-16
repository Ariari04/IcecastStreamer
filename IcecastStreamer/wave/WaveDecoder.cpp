#include "WaveDecoder.h"

#include <fstream>

#include <iostream>
#include <vector>
#include <cstring>


#include <vorbis/vorbisenc.h>

namespace Decoding
{

	//---------------------------------------------


	WaveDecoder::~WaveDecoder()
	{
		close();
	}

	int WaveDecoder::readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration, std::chrono::milliseconds& actualDurationRead)
	{
		int readCount = (this->Header.m_cFormatChunk.m_nChannels * this->Header.m_cFormatChunk.m_nSamplesPerSec * this->Header.m_cFormatChunk.m_nBitsPerSample * duration.count() / 1000) / 8;

		std::cout << "f.tellg() before = " << f.tellg() << std::endl;

		f.read(Buffer, readCount);

		auto bytes = f.gcount();

		std::cout << "f.tellg() after = " << f.tellg() << std::endl;

		actualDurationRead = std::chrono::milliseconds(duration.count() * bytes / readCount);

		return bytes;
	}

	void WaveDecoder::close()
	{
		f.close();
	}

	bool WaveDecoder::open(const char* fileName)
	{

		f.close();

		f.open(fileName, std::ios::binary);

		if (!f)
		{
			return 0;
		}

		size_t nBytesRead = 0;
		size_t nBytesToRead = 0;
		int nBytesToSeek = 0;


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

		while (!f.eof())
		{


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


			f.seekg(tmpHeader.m_nChunkSize, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}


		}

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

		f.read(reinterpret_cast<char*>(&(Header.m_cFormatChunk.m_nFormatTag)), nBytesToRead);

		if (!f)
		{
			close();
			return false;
		}


		if (nBytesToSeek > 0)
		{
			f.seekg(nBytesToSeek, std::ios::cur);

			if (!f)
			{
				close();
				return false;
			}



		}

		// Chunk size is copied here.
		Header.m_cFormatChunk << tmpHeader;
		Header.m_cFormatChunk.m_nChunkSize = 18;

		if (Header.m_cFormatChunk.isOk() == false)
		{
			close();
			return false;
		}

		while (!f.eof())
		{

			f.read(reinterpret_cast<char*>(&tmpHeader), sizeof(WaveFileChunks::ChunkHeader));

			if (!f)
			{
				close();
				return false;
			}

			if (0 == memcmp(tmpHeader.m_id, Header.m_cDataChunk.m_id, 4))
			{
				break;
			}

			f.seekg(tmpHeader.m_nChunkSize, std::ios::cur);

			if (!f)
			{
				close();
			}
		}

		Header.m_cDataChunk << tmpHeader;

		if (f.eof() && (0 != Header.m_cDataChunk.m_id))
		{
			close();
			return false;
		}


		std::cout << "f.tellg() after open = " << f.tellg() << std::endl;

		return true;
	}

	//---------------------------------------------

	void WavToOggConverter::openOutput()
	{
		//fout = std::ofstream("oggtest.ogg", std::ios::out | std::ios::binary);
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

	int WavToOggConverter::finishConvertData(char* Buffer, size_t Count)
	{
		int write = 0;

		vorbis_analysis_wrote(&vd, 0);

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

					//fout.write(reinterpret_cast<char*>(og.header), og.header_len);


					std::copy(og.body, og.body + og.body_len, Buffer + write);

					//fout.write(reinterpret_cast<char*>(og.body), og.body_len);

					//fout.flush();

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

		if (eos == 1)
		{
			ogg_stream_clear(&os);
			vorbis_block_clear(&vb);
			vorbis_dsp_clear(&vd);
			vorbis_comment_clear(&vc);
			vorbis_info_clear(&vi);
		}

		return write;
	}

	int WavToOggConverter::convertData(const char* inputBuffer, size_t inputCount, char* Buffer, size_t Count)
	{
		int write = 0;

		int bytes = inputCount;


		if (dataToSend.size() != 0)
		{
			write = dataToSend.size();
			std::copy(dataToSend.begin(), dataToSend.end(), Buffer);

			//fout.write(&dataToSend[0], dataToSend.size());
			//fout.flush();

			dataToSend.clear();
		}

		//while (!eos)
		{

			float** buffer = vorbis_analysis_buffer(&vd, bytes);

			// uninterleave samples 
			for (int i = 0; i < bytes / 4; i++) {

				buffer[0][i] = ((inputBuffer[i * 4 + 1] << 8) |
					(0x00ff & (int)inputBuffer[i * 4])) / 32768.f;
				buffer[1][i] = ((inputBuffer[i * 4 + 3] << 8) |
					(0x00ff & (int)inputBuffer[i * 4 + 2])) / 32768.f;

				//buffer[0][i] = 0;
				//buffer[1][i] = 0;
			}

			// tell the library how much we actually submitted 
			vorbis_analysis_wrote(&vd, bytes / 4);


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

						//fout.write(reinterpret_cast<char*>(og.header), og.header_len);


						std::copy(og.body, og.body + og.body_len, Buffer + write);

						//fout.write(reinterpret_cast<char*>(og.body), og.body_len);

						//fout.flush();

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

		if (eos == 1)
		{
			ogg_stream_clear(&os);
			vorbis_block_clear(&vb);
			vorbis_dsp_clear(&vd);
			vorbis_comment_clear(&vc);
			vorbis_info_clear(&vi);

			//close();
			//fout.close();

		}

		//int write = lame_encode_buffer_interleaved(lame, &buf[0], buf.size() / this->Header.m_cFormatChunk.m_nChannels, reinterpret_cast<unsigned char*>(Buffer), Count);

		return write;

	}


	//------------------------------------------------

	OggDecoder::~OggDecoder()
	{
		close();
	}

	void OggDecoder::close()
	{
		f.close();
	}

	bool OggDecoder::open(const char* fileName)
	{
		f.close();

		//file = fopen(fileName, "rb");
		f.open(fileName, std::ios::binary);

		if (!f)
		{
			return 0;
		}

		//size_t nBytesRead = 0;
		//size_t nBytesToRead = 0;
		//int nBytesToSeek = 0;

		ogg_int16_t convbuffer[4096]; /* take 8k out of the data segment, not the stack */
		int convsize = 4096;


		//nBytesRead = fread(&(Header.m_cIFFHeader), 1, sizeof(Header.m_cIFFHeader), file);

		//f.read(reinterpret_cast<char*>(&(Header.m_cIFFHeader)), sizeof(Header.m_cIFFHeader));

		char* buffer;
		int  bytes;


		ogg_sync_init(&oy); /* Now we can read pages */




		//int eos = 0;
		int i;

		// grab some data at the head of the stream. We want the first page
		// (which is guaranteed to be small and only contain the Vorbis
		// stream initial header) We need the first page to get the stream
		// serialno.

		// submit a 4k block to libvorbis' Ogg layer 
		buffer = ogg_sync_buffer(&oy, 4096);


		//bytes = fread(buffer, 1, 4096, stdin);
		f.read(buffer, 4096);
		bytes = f.gcount();

		ogg_sync_wrote(&oy, bytes);

		// Get the first page. 
		if (ogg_sync_pageout(&oy, &og) != 1) {
			/* have we simply run out of data?  If so, we're done. */
			if (bytes < 4096)
			{
				ogg_sync_clear(&oy);
				return false;
			}

			/* error case.  Must not be Vorbis data */
			//fprintf(stderr, "Input does not appear to be an Ogg bitstream.\n");
			//exit(1);
			ogg_sync_clear(&oy);
			return false;
		}

		// Get the serial number and set up the rest of decode. 
		// serialno first; use it to set up a logical stream 
		ogg_stream_init(&os, ogg_page_serialno(&og));

		// extract the initial header from the first page and verify that the
		// Ogg bitstream is in fact Vorbis data

		// I handle the initial header first instead of just having the code
		// read all three Vorbis headers at once because reading the initial
		// header is an easy way to identify a Vorbis bitstream and it's
		// useful to see that functionality seperated out.

		vorbis_info_init(&vi);
		vorbis_comment_init(&vc);
		if (ogg_stream_pagein(&os, &og) < 0) {
			// error; stream version mismatch perhaps 
			//fprintf(stderr, "Error reading first page of Ogg bitstream data.\n");
			//exit(1);
			ogg_sync_clear(&oy);
			return false;
		}

		if (ogg_stream_packetout(&os, &op) != 1) {
			// no page? must not be vorbis 
			//fprintf(stderr, "Error reading initial header packet.\n");
			//exit(1);
			ogg_sync_clear(&oy);
			return false;
		}

		if (vorbis_synthesis_headerin(&vi, &vc, &op) < 0) {
			// error case; not a vorbis header
			//fprintf(stderr, "This Ogg bitstream does not contain Vorbis audio data.\n");
			//exit(1);
			ogg_sync_clear(&oy);
			return false;
		}

		// At this point, we're sure we're Vorbis. We've set up the logical
		// (Ogg) bitstream decoder. Get the comment and codebook headers and
		// set up the Vorbis decoder 

		// The next two packets in order are the comment and codebook headers.
		// They're likely large and may span multiple pages. Thus we read
		// and submit data until we get our two packets, watching that no
		// pages are missing. If a page is missing, error out; losing a
		// header page is the only place where missing data is fatal. 

		i = 0;
		while (i < 2) {
			while (i < 2) {
				int result = ogg_sync_pageout(&oy, &og);
				if (result == 0)break; /* Need more data */
				// Don't complain about missing or corrupt data yet. We'll
				// catch it at the packet output phase 
				if (result == 1) {
					ogg_stream_pagein(&os, &og); // we can ignore any errors here
												 // as they'll also become apparent
												 //   at packetout 
					while (i < 2) {
						result = ogg_stream_packetout(&os, &op);
						if (result == 0)break;
						if (result < 0) {
							// Uh oh; data at some point was corrupted or missing!
							// We can't tolerate that in a header.  Die. 
							//fprintf(stderr, "Corrupt secondary header.  Exiting.\n");
							//exit(1);
							return false;
						}
						result = vorbis_synthesis_headerin(&vi, &vc, &op);
						if (result < 0) {
							//fprintf(stderr, "Corrupt secondary header.  Exiting.\n");
							//exit(1);
							return false;
						}
						i++;
					}
				}
			}

			/// no harm in not checking before adding more 
			buffer = ogg_sync_buffer(&oy, 4096);
			//bytes = fread(buffer, 1, 4096, stdin);
			f.read(buffer, 4096);
			bytes = f.gcount();

			if (bytes == 0 && i < 2) {
				//fprintf(stderr, "End of file before finding all Vorbis headers!\n");
				//exit(1);
				return false;
			}
			ogg_sync_wrote(&oy, bytes);
		}

		// Throw the comments plus a few lines about the bitstream we're
		//  decoding
		// {
		//	char** ptr = vc.user_comments;
		//	while (*ptr) {
		//		fprintf(stderr, "%s\n", *ptr);
		//		++ptr;
		//	}
		//	fprintf(stderr, "\nBitstream is %d channel, %ldHz\n", vi.channels, vi.rate);
		//	fprintf(stderr, "Encoded by: %s\n\n", vc.vendor);
		//}

		convsize = 4096 / vi.channels;



		// OK, got and parsed all three headers. Initialize the Vorbis
		// packet->PCM decoder.


		if (vorbis_synthesis_init(&vd, &vi) != 0) {
			//fprintf(stderr, "Error: Corrupt header during playback initialization.\n");

			ogg_stream_clear(&os);
			vorbis_comment_clear(&vc);
			vorbis_info_clear(&vi);  /* must be called last */

			ogg_sync_clear(&oy);

			return false;
		}


		// central decode state
		vorbis_block_init(&vd, &vb);          // local state for most of the decode
												//so multiple block decodes can
												//proceed in parallel. We could init
												//multiple vorbis_block structures
												//for vd here

												// The rest is just a straight decode loop until end of stream




	}

	int OggDecoder::readDuration(char* Buffer, size_t Count, std::chrono::milliseconds duration, std::chrono::milliseconds& actualDurationRead)
	{
		bool keepRunning = true;

		if (eos == 1)
		{
			keepRunning = false;
		}

		char* buffer;
		int  bytes;

		static ogg_int16_t convbuffer[4096];
		int convsize = 4096;

		int writeIndex = 0;

		//int maxCount = 176400;

		int nChannels = 2;

		int nSamplesPerSec = 44100;

		int nBitsPerSample = 16;

		int maxCount = (nChannels * nSamplesPerSec * nBitsPerSample * duration.count() / 1000) / 8;


		int i = 0;


		if (tempBuffer.size() >= maxCount)
		{
			memcpy(Buffer, &tempBuffer[0], maxCount);
			tempBuffer = std::vector<char>(tempBuffer.begin() + maxCount, tempBuffer.end());
			actualDurationRead = duration;
			return maxCount;
		}

		while (keepRunning)
		{
			while (keepRunning)
			{
				int result = ogg_sync_pageout(&oy, &og);
				if (result == 0)break; // need more data 

				if (result < 0) { // missing or corrupt data at this page position 
					//fprintf(stderr, "Corrupt or missing data in bitstream; "
					//	"continuing...\n");



				}
				else {
					ogg_stream_pagein(&os, &og); // can safely ignore errors at this point 


					while (keepRunning) {
						result = ogg_stream_packetout(&os, &op);

						if (result == 0)break; // need more data
						if (result < 0) { //missing or corrupt data at this page position
						  // no reason to complain; already complained above
						}
						else {
							// we have a packet.  Decode it
							float** pcm;
							int samples;

							if (vorbis_synthesis(&vb, &op) == 0) // test for success! 
							{
								vorbis_synthesis_blockin(&vd, &vb);
							}


							// **pcm is a multichannel float vector.  In stereo, for
							// example, pcm[0] is left, and pcm[1] is right.  samples is
							// the size of each channel.  Convert the float values
							// (-1.<=range<=1.) to whatever PCM format and write it out

							while ((samples = vorbis_synthesis_pcmout(&vd, &pcm)) > 0) {
								int j;
								int clipflag = 0;
								int bout = (samples < convsize ? samples : convsize);
								//int bout = (samples < maxCount-writeIndex ? samples : maxCount - writeIndex);

								// convert floats to 16 bit signed ints (host order) and interleave 
								for (i = 0; i < vi.channels; i++) {
									ogg_int16_t* ptr = convbuffer + i;
									//ogg_int16_t* ptr = reinterpret_cast<ogg_int16_t*>(Buffer) + i;
									float* mono = pcm[i];
									for (j = 0; j < bout; j++) {
#if 1
										int val = floor(mono[j] * 32767.f + .5f);
#else // optional dither 
										int val = mono[j] * 32767.f + drand48() - 0.5f;
#endif
										// might as well guard against clipping 
										if (val > 32767) {
											val = 32767;
											clipflag = 1;
										}
										if (val < -32768) {
											val = -32768;
											clipflag = 1;
										}
										*ptr = val;
										ptr += vi.channels;
									}
				        		}

								if (clipflag)
								{
									fprintf(stderr, "Clipping in frame %ld\n", (long)(vd.sequence));
								}


								//fwrite(convbuffer, 2 * vi.channels, bout, stdout);
								//memcpy(&Buffer[writeIndex], convbuffer, bout*2*vi.channels);

								tempBuffer.insert(tempBuffer.end(), reinterpret_cast<char*>(convbuffer), reinterpret_cast<char*>(convbuffer) + bout * 2 * vi.channels);


								//writeIndex += bout * 2 * vi.channels;

								
								vorbis_synthesis_read(&vd, bout); // tell libvorbis how
																  // many samples we
																  // actually consumed 
							}
						}
					}
					if (ogg_page_eos(&og))
					{
						eos = 1;
						keepRunning = false;
					}

					if (tempBuffer.size() >= maxCount)
					{
						keepRunning = false;
					}
				}
				if (tempBuffer.size() >= maxCount)
				{
					keepRunning = false;
				}
			}

			if (tempBuffer.size() >= maxCount)
			{
				keepRunning = false;
			}

			if (!eos) {
				buffer = ogg_sync_buffer(&oy, 4096);
				//bytes = fread(buffer, 1, 4096, stdin);
				f.read(buffer, 4096);
				bytes = f.gcount();

				ogg_sync_wrote(&oy, bytes);
				if (bytes == 0) {
					eos = 1;
					keepRunning = false;
				}
			}
		}


		if (eos)
		{
			vorbis_block_clear(&vb);
			vorbis_dsp_clear(&vd);

			ogg_stream_clear(&os);
			vorbis_comment_clear(&vc);
			vorbis_info_clear(&vi);  /* must be called last */

			/* OK, clean up the framer */
			ogg_sync_clear(&oy);
		}

		if (tempBuffer.size() >= maxCount)
		{
			memcpy(Buffer, &tempBuffer[0], maxCount);
			tempBuffer = std::vector<char>(tempBuffer.begin() + 176400, tempBuffer.end());
			actualDurationRead = duration;
			return maxCount;
		}
		else
		{
			auto size = tempBuffer.size();
			if (size > 0)
			{
				memcpy(Buffer, &tempBuffer[0], size);
				tempBuffer.clear();
			}
			actualDurationRead = duration * size / maxCount;
			return size;
		}
	}
}
