/*
*   Copyright (C) 2017,2019 by Jonathan Naylor G4KLX
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

#if !defined(AMBE2WAV_H)
#define	AMBE2WAV_H

#include "DV3000SerialController.h"

#include <string>

class CAMBE2WAV
{
public:
	CAMBE2WAV(const std::string& signature, AMBE_MODE mode, bool fec, const std::string& port, unsigned int speed, float amplitude, bool reset, bool debug, const std::string& input, const std::string& output);
	~CAMBE2WAV();

	int run();

private:
	std::string  m_signature;
	AMBE_MODE    m_mode;
	bool         m_fec;
	std::string  m_port;
	unsigned int m_speed;
	float        m_amplitude;
	bool         m_reset;
	bool         m_debug;
	std::string  m_input;
	std::string  m_output;
};

#endif
