/*
 *   Copyright (C) 2002-2004,2006-2009,2017 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "WAVFileWriter.h"

#include <cassert>

#if defined(_WIN32) || defined(_WIN64)

const int WAVE_FORMAT_IEEE_FLOAT = 3;

CWAVFileWriter::CWAVFileWriter(const std::string& fileName, unsigned int sampleRate, unsigned int channels, unsigned int sampleWidth, unsigned int blockSize) :
m_fileName(fileName),
m_sampleRate(sampleRate),
m_channels(channels),
m_sampleWidth(sampleWidth),
m_blockSize(blockSize),
m_buffer8(NULL),
m_buffer16(NULL),
m_buffer32(NULL),
m_handle(NULL),
m_parent(),
m_child()
{
	assert(sampleRate > 0U);
	assert(channels == 1U || channels == 2U);
	assert(sampleWidth == 8U || sampleWidth == 16U || sampleWidth == 32U);
	assert(blockSize > 0U);

	m_buffer8  = new uint8_t[blockSize * channels];
	m_buffer16 = new int16_t[blockSize * channels];
	m_buffer32 = new float[blockSize * channels];
}

CWAVFileWriter::~CWAVFileWriter()
{
	delete[] m_buffer8;
	delete[] m_buffer16;
	delete[] m_buffer32;
}

bool CWAVFileWriter::open()
{
	m_handle = ::mmioOpen(LPSTR(m_fileName.c_str()), 0, MMIO_WRITE | MMIO_CREATE | MMIO_ALLOCBUF);
	if (m_handle == NULL) {
		::fprintf(stderr, "WAVFileWriter: could not open the file %s\n", m_fileName.c_str());
		return false;
	}

	m_parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	m_parent.cksize  = 0;

	MMRESULT res = ::mmioCreateChunk(m_handle, &m_parent, MMIO_CREATERIFF);
	if (res != MMSYSERR_NOERROR) {
		::fprintf(stderr, "WAVFileWriter: could not write to file %s\n", m_fileName.c_str());
		return false;
	}

	m_child.ckid   = mmioFOURCC('f', 'm', 't', ' ');
	m_child.cksize = sizeof(WAVEFORMATEX);

	res = ::mmioCreateChunk(m_handle, &m_child, 0);
	if (res != MMSYSERR_NOERROR) {
		::fprintf(stderr, "WAVFileWriter: could not write to the file %s\n", m_fileName.c_str());
		return false;
	}

	WAVEFORMATEX format;
	format.wBitsPerSample  = m_sampleWidth;
	if (m_sampleWidth == 8U || m_sampleWidth == 16U)
		format.wFormatTag = WAVE_FORMAT_PCM;
	else
		format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	format.nChannels       = m_channels;
	format.nSamplesPerSec  = m_sampleRate;
	format.nAvgBytesPerSec = m_sampleRate * m_channels * m_sampleWidth / 8;
	format.nBlockAlign     = m_channels * m_sampleWidth / 8;
	format.cbSize          = 0;

	LONG n = ::mmioWrite(m_handle, (CHAR *)&format, sizeof(WAVEFORMATEX));
	if (n != sizeof(WAVEFORMATEX)) {
		::fprintf(stderr, "WAVFileWriter: could not write to the file %s\n", m_fileName.c_str());
		return false;
	}

	::mmioAscend(m_handle, &m_child, 0);

	m_child.ckid   = mmioFOURCC('d', 'a', 't', 'a');
	m_child.cksize = 0;

	res = ::mmioCreateChunk(m_handle, &m_child, 0);
	if (res != MMSYSERR_NOERROR) {
		::fprintf(stderr, "WAVFileWriter: could not write to the file %s\n", m_fileName.c_str());
		return false;
	}

	return true;
}

bool CWAVFileWriter::write(const float* buffer, unsigned int length)
{
	assert(m_handle != NULL);
	assert(buffer != NULL);
	assert(length > 0U);

	unsigned int i;
	LONG bytes = 0L;
	LONG n = 0L;

	switch (m_sampleWidth) {
		case 8U:
			switch (m_channels) {
				case 1U:
					for (i = 0U; i < length; i++)
						m_buffer8[i] = uint8_t(buffer[i] * 128.0F + 127.0F);
					break;
				case 2U:
					for (i = 0U; i < (length * 2U); i++)
						m_buffer8[i] = uint8_t(buffer[i] * 128.0F + 127.0F);
					break;
			}

			bytes = length * m_channels * sizeof(uint8_t);

			n = ::mmioWrite(m_handle, (char *)m_buffer8, bytes);

			break;

		case 16U:
			switch (m_channels) {
				case 1U:
					for (i = 0U; i < length; i++)
						m_buffer16[i] = int16_t(buffer[i] * 32768.0F);
					break;
				case 2U:
					for (i = 0U; i < (length * 2U); i++)
						m_buffer16[i] = int16_t(buffer[i] * 32768.0F);
					break;
			}

			bytes = length * m_channels * sizeof(int16_t);

			n = ::mmioWrite(m_handle, (char *)m_buffer16, bytes);

			break;

		case 32U:
			switch (m_channels) {
				case 1U:
					for (i = 0U; i < length; i++)
						m_buffer32[i] = float(buffer[i]);
					break;
				case 2U:
					// Swap I and Q
					for (i = 0U; i < length; i++) {
						m_buffer32[i * 2U + 0U] = float(buffer[i * 2U + 1U]);
						m_buffer32[i * 2U + 1U] = float(buffer[i * 2U + 0U]);
					}
					break;
			}

			bytes = length * m_channels * sizeof(float);

			n = ::mmioWrite(m_handle, (char *)m_buffer32, bytes);

			break;
	}

	return n == bytes;
}

void CWAVFileWriter::close()
{
	assert(m_handle != NULL);

	::mmioAscend(m_handle, &m_child, 0);
	::mmioAscend(m_handle, &m_parent, 0);

	::mmioClose(m_handle, 0);
	m_handle = NULL;
}

#else

CWAVFileWriter::CWAVFileWriter(const std::string& fileName, unsigned int sampleRate, unsigned int channels, unsigned int sampleWidth, unsigned int blockSize) :
m_fileName(fileName),
m_sampleRate(sampleRate),
m_channels(channels),
m_sampleWidth(sampleWidth),
m_blockSize(blockSize),
m_buffer8(NULL),
m_buffer16(NULL),
m_buffer32(NULL),
m_file(NULL),
m_offset1(0),
m_offset2(0),
m_length(0U)
{
	assert(sampleRate > 0U);
	assert(channels == 1U || channels == 2U);
	assert(sampleWidth == 8U || sampleWidth == 16U || sampleWidth == 32U);
	assert(blockSize > 0U);

	m_buffer8  = new uint8_t[channels * blockSize];
	m_buffer16 = new int16_t[channels * blockSize];
	m_buffer32 = new float[channels * blockSize];
}

CWAVFileWriter::~CWAVFileWriter()
{
	delete[] m_buffer8;
	delete[] m_buffer16;
	delete[] m_buffer32;
}

bool CWAVFileWriter::open()
{
	m_length = 0U ;

	m_file = ::fopen(m_fileName.c_str(), "wb");
	if (m_file == NULL) {
		::fprintf(stderr, "WAVFileWriter: could not open the file %s in WAVFileWriter\n", m_fileName.c_str());
		return false;
	}

	::fwrite("RIFF", sizeof(uint8_t), 4, m_file);   // 4 bytes, file signature

	m_offset1 = ::ftell(m_file);

	uint32_t uint32 = 0;
	::fwrite(&uint32, sizeof(uint32_t), 1, m_file); // 4 bytes, length of file, filled in later

	::fwrite("WAVE", sizeof(uint8_t), 4, m_file);   // 4 bytes, RIFF file type

	::fwrite("fmt ", sizeof(uint8_t), 4, m_file);   // 4 bytes, chunk signature

	uint32 = 16U;
	::fwrite(&uint32, sizeof(uint32_t), 1, m_file); // 4 bytes, length of "fmt " chunk

	uint16_t uint16;
	if (m_sampleWidth == 8U || m_sampleWidth == 16U)
		uint16 = 1U;                                // 2 bytes, integer PCM/uncompressed
	else
		uint16 = 3U;                                // 2 bytes, float PCM/uncompressed
	::fwrite(&uint16, sizeof(uint16), 1, m_file);

	::fwrite(&m_channels, sizeof(uint16), 1, m_file);// 2 bytes, no of channels
        
	::fwrite(&m_sampleRate, sizeof(uint32_t), 1, m_file);// 4 bytes, sample rate

	uint32 = m_sampleRate * m_channels * m_sampleWidth / 8U;
	::fwrite(&uint32, sizeof(uint32_t), 1, m_file); // 4 bytes, average bytes per second

	uint16 = m_channels * m_sampleWidth / 8U;
	::fwrite(&uint16, sizeof(uint16), 1, m_file);   // 2 bytes, block alignment

	::fwrite(&m_sampleWidth, sizeof(uint16), 1, m_file); // 2 bytes, significant bits per sample

	::fwrite("data", 4, 1, m_file);                      // 4 bytes, chunk signature

	m_offset2 = ::ftell(m_file);

	uint32 = 0U;
	::fwrite(&uint32, sizeof(uint32_t), 1, m_file);      // 4 bytes, length of "data" chunk, filled in later

	return true;
}

bool CWAVFileWriter::write(const float* buffer, unsigned int length)
{
	assert(m_file != NULL);
	assert(buffer != NULL);
	assert(length > 0U && length <= m_blockSize);

	unsigned int bytes = 0U;
	unsigned int i;
	size_t n = 0UL;

	switch (m_sampleWidth) {
		case 8U:
			switch (m_channels) {
				case 1U:
					for (i = 0U; i < length; i++)
						m_buffer8[i] = uint8_t(buffer[i] * 128.0F + 127.0F);
					break;
				case 2U:
					for (i = 0U; i < (length * 2U); i++)
						m_buffer8[i] = uint8_t(buffer[i] * 128.0F + 127.0F);
					break;
			}

			bytes = length * m_channels;

			n = ::fwrite(m_buffer8, sizeof(uint8_t), bytes, m_file);

			break;

		case 16U:
			switch (m_channels) {
				case 1U:
					for (i = 0U; i < length; i++)
						m_buffer16[i] = int16_t(buffer[i] * 32768.0F);
					break;
				case 2U:
					for (i = 0U; i < (length * 2U); i++)
						m_buffer16[i] = int16_t(buffer[i] * 32768.0F);
					break;
			}

			bytes = length * m_channels;

			n = ::fwrite(m_buffer16, sizeof(uint16_t), bytes, m_file);

			break;

		case 32U:
			switch (m_channels) {
				case 1U:
					for (i = 0U; i < length; i++)
						m_buffer32[i] = float(buffer[i]);
					break;
				case 2U:
					for (i = 0U; i < length; i++) {
						m_buffer32[i * 2U + 0U] = float(buffer[i * 2U + 0U]);
						m_buffer32[i * 2U + 1U] = float(buffer[i * 2U + 1U]);
					}
					break;
			}

			bytes = length * m_channels;

			n = ::fwrite(m_buffer32, sizeof(float), bytes, m_file);

			break;
	}

	m_length += n;

	return n == bytes;
}

void CWAVFileWriter::close()
{
	assert(m_file != NULL);

	if ((m_length % 2U) != 0U) {
		unsigned char c = 0U;
		::fwrite(&c, sizeof(uint8_t), 1, m_file);
	}

	::fseek(m_file, m_offset2, SEEK_SET);
	::fwrite(&m_length, sizeof(uint32_t), 1, m_file);

	uint32_t length = m_length + 36U;

	::fseek(m_file, m_offset1, SEEK_SET);
	::fwrite(&length, sizeof(uint32_t), 1, m_file);

	::fclose(m_file);
	m_file = NULL;
}

#endif
