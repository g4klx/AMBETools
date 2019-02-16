/*
 *   Copyright (C) 2002-2004,2006-2010,2017,2019 by Jonathan Naylor G4KLX
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

#ifndef	WAVFileWriter_H
#define WAVFileWriter_H

#include <string>

#include <cstdio>
#include <sndfile.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <mmsystem.h>
#endif

class CWAVFileWriter {
public:
	CWAVFileWriter(const std::string& fileName, unsigned int sampleRate, unsigned int channels, unsigned int sampleWidth, unsigned int blockSize);
	~CWAVFileWriter();

	bool open();
	bool write(const float* buffer, unsigned int length);
	void close();

private:
	std::string    m_fileName;
	unsigned int   m_sampleRate;
	unsigned short m_channels;
	unsigned short m_sampleWidth;
	unsigned int   m_blockSize;
#if defined(_WIN32) || defined(_WIN64)
	uint8_t*       m_buffer8;
	int16_t*       m_buffer16;
	HMMIO          m_handle;
	MMCKINFO       m_parent;
	MMCKINFO       m_child;
#else
	SNDFILE*       m_file;
#endif
};

#endif
