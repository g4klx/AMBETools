/*
*   Copyright (C) 2017,2019,2023 by Jonathan Naylor G4KLX
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

#if !defined(AMBE2DVTOOL_H)
#define	AMBE2DVTOOL_H

#include <string>

class CAMBE2DVTOOL
{
public:
	CAMBE2DVTOOL(const std::string& signature, bool debug, const std::string& input, const std::string& output);
	~CAMBE2DVTOOL();

	int run();

private:
	std::string m_signature;
	bool        m_debug;
	std::string m_input;
	std::string m_output;
};

#endif
