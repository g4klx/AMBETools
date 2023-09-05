/*
 *   Copyright (C) 2009,2010,2014,2023 by Jonathan Naylor G4KLX
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

#ifndef	DVTOOLFileWriter_H
#define DVTOOLFileWriter_H

#include <cstdio>
#include <cstdint>

#include <string>

class CDVTOOLFileWriter {
public:
	CDVTOOLFileWriter(const std::string& filename);
	~CDVTOOLFileWriter();

	bool open();

	bool write(const uint8_t* buffer, unsigned int length);

	void close();

private:
	std::string m_filename;
	FILE*       m_file;
	uint32_t    m_count;
	uint8_t     m_sequence;
	long        m_offset;

	bool writeHeader();
	bool writeHeader(const uint8_t* buffer, unsigned int length);
	bool writeTrailer();

	uint16_t uint16SwapOnBE(uint16_t value) const;
	uint32_t uint32SwapOnLE(uint32_t value) const;
};

#endif
