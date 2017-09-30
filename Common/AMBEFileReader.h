/*
*   Copyright (C) 2017 by Jonathan Naylor G4KLX
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

#ifndef	AMBEFileReader_H
#define AMBEFileReader_H

#include <string>

#include <cstdio>

class CAMBEFileReader {
public:
	CAMBEFileReader(const std::string& fileName, const std::string& signature);
	~CAMBEFileReader();

	bool         open();
	unsigned int read(uint8_t* data, unsigned int length);
	void         close();

private:
	std::string m_fileName;
	std::string m_signature;
	FILE*       m_fp;
};

#endif
