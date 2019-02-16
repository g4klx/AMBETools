/*
 *   Copyright (C) 2002-2004,2006-2009,2014,2017,2019 by Jonathan Naylor G4KLX
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

#include "WAVFileReader.h"

#include <cassert>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)

const int WAVE_FORMAT_IEEE_FLOAT = 3;

CWAVFileReader::CWAVFileReader(const std::string& fileName, unsigned int blockSize) :
m_fileName(fileName),
m_blockSize(blockSize),
m_channels(0U),
m_sampleRate(0U),
m_format(FORMAT_16BIT),
m_buffer8(NULL),
m_buffer16(NULL),
m_handle(NULL),
m_parent(),
m_child(),
m_offset(0L)
{
	assert(blockSize > 0U);

	m_buffer8  = new uint8_t[blockSize * 4U];
	m_buffer16 = new int16_t[blockSize * 4U];
}

CWAVFileReader::~CWAVFileReader()
{
	delete[] m_buffer8;
	delete[] m_buffer16;
}

bool CWAVFileReader::open()
{
	m_handle = ::mmioOpen(LPSTR(m_fileName.c_str()), 0, MMIO_READ | MMIO_ALLOCBUF);
	if (m_handle == NULL) {
		::fprintf(stderr, "WAVFileReader: could not open the WAV file %s\n", m_fileName.c_str());
		return false;
	}

	MMCKINFO parent;
	parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');

	MMRESULT res = ::mmioDescend(m_handle, &parent, 0, MMIO_FINDRIFF);
	if (res != MMSYSERR_NOERROR) {
		::fprintf(stderr, "WAVFileReader: %s has no \"WAVE\" header\n", m_fileName.c_str());
		::mmioClose(m_handle, 0U);
		m_handle = NULL;
		return false;
	}

	MMCKINFO child;
	child.ckid = mmioFOURCC('f', 'm', 't', ' ');

	res = ::mmioDescend(m_handle, &child, &parent, MMIO_FINDCHUNK);
	if (res != MMSYSERR_NOERROR) {
		::fprintf(stderr, "WAVFileReader: %s has no \"fmt \" chunk\n", m_fileName.c_str());
		::mmioClose(m_handle, 0U);
		m_handle = NULL;
		return false;
	}

	WAVEFORMATEX format;

	LONG len = ::mmioRead(m_handle, (char *)&format, child.cksize);
	if (len != LONG(child.cksize)) {
		::fprintf(stderr, "WAVFileReader: %s is corrupt, cannot read the WAVEFORMATEX structure\n", m_fileName.c_str());
		::mmioClose(m_handle, 0U);
		m_handle = NULL;
		return false;
	}
        
	if (format.wFormatTag != WAVE_FORMAT_PCM && format.wFormatTag != WAVE_FORMAT_IEEE_FLOAT) {
		::fprintf(stderr, "WAVFileReader: %s is not PCM or IEEE Float format, is %u\n", m_fileName.c_str(), format.wFormatTag);
		::mmioClose(m_handle, 0U);
		m_handle = NULL;
		return false;
	}

	m_sampleRate = format.nSamplesPerSec;

	m_channels = format.nChannels;
	if (m_channels > 2U) {
		::fprintf(stderr, "WAVFileReader: %s has %u channels, more than 2\n", m_fileName.c_str(), m_channels);
		::mmioClose(m_handle, 0U);
		m_handle = NULL;
		return false;
	}

	if (format.wBitsPerSample == 8U && format.wFormatTag == WAVE_FORMAT_PCM) {
		m_format = FORMAT_8BIT;
	} else if (format.wBitsPerSample == 16U && format.wFormatTag == WAVE_FORMAT_PCM) {
		m_format = FORMAT_16BIT;
	} else if (format.wBitsPerSample == 32U && format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
		m_format = FORMAT_32BIT;
	} else {
		::fprintf(stderr, "WAVFileReader: %s has sample width %u and format %u\n", m_fileName.c_str(), format.wBitsPerSample, format.wFormatTag);
		::mmioClose(m_handle, 0U);
		m_handle = NULL;
		return false;
	}

	res = ::mmioAscend(m_handle, &child, 0);
	if (res != MMSYSERR_NOERROR) {
		::fprintf(stderr, "WAVFileReader: %s is corrupt, cannot ascend\n", m_fileName.c_str());
		::mmioClose(m_handle, 0U);
		m_handle = NULL;
		return false;
	}

	child.ckid = mmioFOURCC('d', 'a', 't', 'a');

	res = ::mmioDescend(m_handle, &child, &parent, MMIO_FINDCHUNK);
	if (res != MMSYSERR_NOERROR) {
		::fprintf(stderr, "WAVFileReader: %s has no \"data\" chunk\n", m_fileName.c_str());
		::mmioClose(m_handle, 0U);
		m_handle = NULL;
		return false;
	}

	// Get the current location so we can rewind if needed
	m_offset = ::mmioSeek(m_handle, 0L, SEEK_CUR);

	return true;
}

unsigned int CWAVFileReader::read(float* data, unsigned int length)
{
	assert(m_handle != NULL);
	assert(data != NULL);

	if (length == 0U)
		return 0U;

	unsigned int elements = length * m_channels;
	LONG n = 0L;
	LONG i;

	switch (m_format) {
		case FORMAT_8BIT:
			n = ::mmioRead(m_handle, (char *)m_buffer8, elements * sizeof(uint8_t));

			if (n <= 0L)
				return 0U;

			n /= sizeof(uint8_t);

			for (i = 0L; i < n; i++)
				data[i] = (float(m_buffer8[i]) - 127.0F) / 128.0F;

			break;

		case FORMAT_16BIT:
			n = ::mmioRead(m_handle, (char *)m_buffer16, elements * sizeof(int16_t));

			if (n <= 0L)
				return 0U;

			n /= sizeof(int16_t);

			for (i = 0L; i < n; i++)
				data[i] = float(m_buffer16[i]) / 32768.0F;

			break;

		case FORMAT_32BIT:
			n = ::mmioRead(m_handle, (char *)data, elements * sizeof(float));

			if (n <= 0L)
				return 0U;

			n /= sizeof(float);

			break;
	}

	return n / m_channels;
}

void CWAVFileReader::rewind()
{
	assert(m_handle != NULL);

	::mmioSeek(m_handle, m_offset, SEEK_SET);
}

void CWAVFileReader::close()
{
	assert(m_handle != NULL);

	::mmioClose(m_handle, 0U);

	m_handle = NULL;
}

unsigned int CWAVFileReader::getSampleRate() const
{
	return m_sampleRate;
}

unsigned int CWAVFileReader::getChannels() const
{
	return m_channels;
}

#else

const int FORMAT_PCM        = 1;
const int FORMAT_IEEE_FLOAT = 3;

CWAVFileReader::CWAVFileReader(const std::string& fileName, unsigned int blockSize) :
m_fileName(fileName),
m_blockSize(blockSize),
m_channels(0U),
m_sampleRate(0U),
m_file(NULL)
{
	assert(blockSize > 0U);
}

CWAVFileReader::~CWAVFileReader()
{
}

bool CWAVFileReader::open()
{
	SF_INFO info;
	info.format = 0;

	m_file = ::sf_open(m_fileName.c_str(), SFM_READ, &info);
	if (m_file == NULL) {
		::fprintf(stderr, "WAVFileReader: could not open the WAV file %s.\n", m_fileName.c_str());
		return false;
	}

	m_channels   = info.channels;
	m_sampleRate = info.samplerate;

	return true;
}

unsigned int CWAVFileReader::read(float* data, unsigned int length)
{
	assert(m_file != NULL);
	assert(data != NULL);

	if (length == 0U)
		return 0U;

	unsigned int elements = length * m_channels;

	sf_count_t n = ::sf_readf_float(m_file, data, elements);
	if (n == 0U)
		return 0U;

	return n / m_channels;
}

void CWAVFileReader::rewind()
{
	assert(m_file != NULL);

	::sf_seek(m_file, 0, SEEK_SET);
}

void CWAVFileReader::close()
{
	assert(m_file != NULL);

	::sf_close(m_file);

	m_file = NULL;
}

unsigned int CWAVFileReader::getSampleRate() const
{
	return m_sampleRate;
}

unsigned int CWAVFileReader::getChannels() const
{
	return m_channels;
}

#endif
