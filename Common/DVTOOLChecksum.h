/*
 *	Copyright (C) 2009,2011,2014,2023 by Jonathan Naylor, G4KLX
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 */

#ifndef	DVTOOLChecksum_H
#define	DTOOLChecksum_H

#include <cstdint>

class CDVTOOLChecksum {
public:
	CDVTOOLChecksum();
	~CDVTOOLChecksum();

	void update(const uint8_t* data, unsigned int length);

	void result(uint8_t* data);

	bool check(const uint8_t* data);

	void reset();

private:
	union {
		uint16_t m_crc16;
		uint8_t  m_crc8[2U];
	};
};

#endif
