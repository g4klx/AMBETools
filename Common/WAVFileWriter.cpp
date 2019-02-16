/*
 *   Copyright (C) 2002-2004,2006-2009,2017,2019 by Jonathan Naylor G4KLX
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
}

CWAVFileWriter::~CWAVFileWriter()
{
	delete[] m_buffer8;
	delete[] m_buffer16;
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

	unsigned int elements = length * m_channels;
	unsigned int i;
	LONG bytes = 0L;
	LONG n = 0L;

	switch (m_sampleWidth) {
		case 8U:
			for (i = 0U; i < elements; i++)
				m_buffer8[i] = uint8_t(buffer[i] * 128.0F + 127.0F);

			bytes = elements * sizeof(uint8_t);

			n = ::mmioWrite(m_handle, (char *)m_buffer8, bytes);

			break;

		case 16U:
			for (i = 0U; i < elements; i++)
				m_buffer16[i] = int16_t(buffer[i] * 32768.0F);

			bytes = elements * sizeof(int16_t);

			n = ::mmioWrite(m_handle, (char *)m_buffer16, bytes);

			break;

		case 32U:
			bytes = elements * sizeof(float);

			n = ::mmioWrite(m_handle, (char *)buffer, bytes);

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
m_file(NULL)
{
	assert(sampleRate > 0U);
	assert(channels == 1U || channels == 2U);
	assert(sampleWidth == 8U || sampleWidth == 16U || sampleWidth == 32U);
	assert(blockSize > 0U);
}

CWAVFileWriter::~CWAVFileWriter()
{
}

bool CWAVFileWriter::open()
{
	SF_INFO info;
	info.samplerate = m_sampleRate;
	info.channels   = m_channels;
	info.format     = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

	m_file = ::sf_open(m_fileName.c_str(), SFM_WRITE, &info);
	if (m_file == NULL) {
		::fprintf(stderr, "WAVFileWriter: could not open the file %s in WAVFileWriter\n", m_fileName.c_str());
		return false;
	}

	return true;
}

bool CWAVFileWriter::write(const float* buffer, unsigned int length)
{
	assert(m_file != NULL);
	assert(buffer != NULL);
	assert(length > 0U && length <= m_blockSize);

	unsigned int elements = length * m_channels;

	sf_count_t n = ::sf_writef_float(m_file, buffer, elements);

	return n == elements;
}

void CWAVFileWriter::close()
{
	assert(m_file != NULL);

	::sf_close(m_file);

	m_file = NULL;
}

#endif
