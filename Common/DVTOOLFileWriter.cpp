/*
 *   Copyright (C) 2009,2010,2012,2014,2023 by Jonathan Naylor G4KLX
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

#include "DVTOOLFileWriter.h"
#include "DVTOOLChecksum.h"

#include <cassert>
#include <cstring>

const uint8_t DVTOOL_SIGNATURE[] = "DVTOOL";
const size_t  DVTOOL_SIGNATURE_LENGTH = 6U;

const uint8_t DSVT_SIGNATURE[] = "DSVT";
const size_t  DSVT_SIGNATURE_LENGTH = 4U;

const uint8_t HEADER_FLAG = 0x10;
const uint8_t DATA_FLAG   = 0x20;

const uint8_t FIXED_DATA[] = {0x00, 0x81, 0x00, 0x20, 0x00, 0x01, 0x02, 0xC0, 0xDE};
const size_t  FIXED_DATA_LENGTH = 9U;

const uint8_t TRAILER_DATA[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const size_t  TRAILER_DATA_LENGTH = 12U;

const uint8_t HEADER_MASK  = 0x80;
const uint8_t TRAILER_MASK = 0x40;

const size_t LONG_CALLSIGN_LENGTH      = 8U;
const size_t SHORT_CALLSIGN_LENGTH     = 4U;
const size_t RADIO_HEADER_LENGTH_BYTES = 41U;

CDVTOOLFileWriter::CDVTOOLFileWriter(const std::string& filename) :
m_filename(filename),
m_file(),
m_count(0U),
m_sequence(0U),
m_offset(0)
{
}

CDVTOOLFileWriter::~CDVTOOLFileWriter()
{
}

bool CDVTOOLFileWriter::open()
{
	m_file = ::fopen(m_filename.c_str(), "wb");
	if (m_file == NULL)
		return false;

	::fwrite(DVTOOL_SIGNATURE, DVTOOL_SIGNATURE_LENGTH, 1U, m_file);

	m_offset = ::ftell(m_file);

	uint32_t dummy = 0U;
	::fwrite(&dummy, sizeof(uint32_t), 1U, m_file);

	m_sequence = 0U;
	m_count = 0U;

	return writeHeader();
}

bool CDVTOOLFileWriter::writeHeader()
{
	uint8_t buffer[RADIO_HEADER_LENGTH_BYTES];
	::memset(buffer, ' ', RADIO_HEADER_LENGTH_BYTES);

	buffer[0] = 0x00U;
	buffer[1] = 0x00U;
	buffer[2] = 0x00U;

	// Get the checksum for the header
	CDVTOOLChecksum csum;
	csum.update(buffer + 0U, 4U * LONG_CALLSIGN_LENGTH + SHORT_CALLSIGN_LENGTH + 3U);
	csum.result(buffer + 4U * LONG_CALLSIGN_LENGTH + SHORT_CALLSIGN_LENGTH + 3U);

	return writeHeader(buffer, RADIO_HEADER_LENGTH_BYTES);
}

bool CDVTOOLFileWriter::writeHeader(const uint8_t* buffer, unsigned int length)
{
	assert(buffer != NULL);
	assert(length > 0U);

	// uint16_t little-endian
	uint16_t len = uint16SwapOnBE(length + 15U);
	::fwrite(&len, sizeof(uint16_t), 1U, m_file);

	::fwrite(DSVT_SIGNATURE, DSVT_SIGNATURE_LENGTH, 1U, m_file);

	uint8_t byte = HEADER_FLAG;
	::fwrite(&byte, sizeof(uint8_t), 1U, m_file);

	::fwrite(FIXED_DATA, FIXED_DATA_LENGTH, 1U, m_file);

	byte = HEADER_MASK;
	::fwrite(&byte, sizeof(uint8_t), 1U, m_file);

	::fwrite(buffer, length, 1U, m_file);

	m_count++;

	return true;
}

bool CDVTOOLFileWriter::write(const uint8_t* buffer, unsigned int length)
{
	assert(buffer != NULL);
	assert(length > 0U);

	// uint16_t little-endian
	uint16_t len = uint16SwapOnBE(length + 15U);
	::fwrite(&len, sizeof(uint16_t), 1U, m_file);

	::fwrite(DSVT_SIGNATURE, DSVT_SIGNATURE_LENGTH, 1U, m_file);

	uint8_t byte = DATA_FLAG;
	::fwrite(&byte, sizeof(uint8_t), 1U, m_file);

	::fwrite(FIXED_DATA, FIXED_DATA_LENGTH, 1U, m_file);

	byte = m_sequence;
	::fwrite(&byte, sizeof(uint8_t), 1U, m_file);

	::fwrite(buffer, length, 1U, m_file);

	m_count++;
	m_sequence++;
	if (m_sequence >= 0x15U)
		m_sequence = 0U;

	return true;
}

void CDVTOOLFileWriter::close()
{
	writeTrailer();

	::fseek(m_file, m_offset, SEEK_SET);

	// uint32_t big-endian
	uint32_t count = uint32SwapOnLE(m_count);
	::fwrite(&count, sizeof(uint32_t), 1U, m_file);

	::fclose(m_file);
	m_file = NULL;
}

bool CDVTOOLFileWriter::writeTrailer()
{
	// uint16_t little-endian
	uint16_t len = uint16SwapOnBE(27U);
	::fwrite(&len, sizeof(uint16_t), 1U, m_file);

	::fwrite(DSVT_SIGNATURE, DSVT_SIGNATURE_LENGTH, 1U, m_file);

	uint8_t byte = DATA_FLAG;
	::fwrite(&byte, sizeof(uint8_t), 1U, m_file);

	::fwrite(FIXED_DATA, FIXED_DATA_LENGTH, 1U, m_file);

	byte = TRAILER_MASK | m_sequence;
	::fwrite(&byte, sizeof(uint8_t), 1U, m_file);

	::fwrite(TRAILER_DATA, TRAILER_DATA_LENGTH, 1U, m_file);

	return true;
}

uint16_t CDVTOOLFileWriter::uint16SwapOnBE(uint16_t value) const
{
// GNU specific
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	return ((value & 0x00FFU) << 8) |
	       ((value & 0xFF00U) >> 8);
#else
	return value;
#endif
}

uint32_t CDVTOOLFileWriter::uint32SwapOnLE(uint32_t value) const
{
// GNU specific
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return ((value & 0xFF000000U) >> 24) |
	       ((value & 0x00FF0000U) >> 8) |
	       ((value & 0x0000FF00U) << 8) |
	       ((value & 0x000000FFU) << 24);
#else
	return value;
#endif
}

