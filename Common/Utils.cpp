/*
*   Copyright (C) 2009,2014,2017,2019 by Jonathan Naylor G4KLX
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

#include "Utils.h"

#include <string>

#include <cstdio>
#include <cassert>

void CUtils::dump(const char* title, const unsigned char* data, unsigned int length)
{
	assert(title != NULL);
	assert(data != NULL);

	::fprintf(stdout, "%s\n", title);

	unsigned int offset = 0U;

	while (length > 0U) {
		std::string output;

		unsigned int bytes = (length > 16U) ? 16U : length;

		for (unsigned i = 0U; i < bytes; i++) {
			char temp[10U];
			::sprintf(temp, "%02X ", data[offset + i]);
			output += temp;
		}

		for (unsigned int i = bytes; i < 16U; i++)
			output += "   ";

		output += "   *";

		for (unsigned i = 0U; i < bytes; i++) {
			unsigned char c = data[offset + i];

			if (::isprint(c))
				output += c;
			else
				output += '.';
		}

		output += '*';

		::fprintf(stdout, "%04X:  %s\n", offset, output.c_str());

		offset += 16U;

		if (length >= 16U)
			length -= 16U;
		else
			length = 0U;
	}
}

#if defined(_WIN32) || defined(_WIN64)

#include <Windows.h>

void CUtils::sleep(unsigned int ms)
{
	::Sleep(ms);
}

#else

#include <unistd.h>

void CUtils::sleep(unsigned int ms)
{
	::usleep(ms * 1000);
}

#endif
