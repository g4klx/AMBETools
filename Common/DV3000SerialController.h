/*
 *   Copyright (C) 2014,2015,2017,2019 by Jonathan Naylor G4KLX
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

#ifndef	DV3000SerialController_H
#define	DV3000SerialController_H

#include "WAVFileReader.h"
#include "WAVFileWriter.h"
#include "AMBEFileReader.h"
#include "AMBEFileWriter.h"
#include "SerialController.h"

#include <string>

enum AMBE_MODE {
	MODE_DSTAR,
	MODE_DMR,
	MODE_P25,
	MODE_UNKNOWN
};

const unsigned int AUDIO_SAMPLE_RATE = 8000U;
const unsigned int AUDIO_BLOCK_SIZE = AUDIO_SAMPLE_RATE / 50U;

class CDV3000SerialController {
public:
	CDV3000SerialController(const std::string& device, unsigned int speed, AMBE_MODE mode, bool fec, float amplitude, bool reset, bool debug, CAMBEFileReader* reader, CWAVFileWriter* writer);
	CDV3000SerialController(const std::string& device, unsigned int speed, AMBE_MODE mode, bool fec, float amplitude, bool reset, bool debug, CWAVFileReader* reader, CAMBEFileWriter* writer);
	~CDV3000SerialController();

	bool open();

	void process();

	void close();

private:
	enum AMBE_DIRECTION {
		AMBE_ENCODING,
		AMBE_DECODING
	};

	CSerialController m_serial;
	AMBE_MODE         m_mode;
	bool              m_fec;
	float             m_amplitude;
	bool              m_reset;
	bool              m_debug;
	AMBE_DIRECTION    m_direction;
	CWAVFileReader*   m_wavReader;
	CWAVFileWriter*   m_wavWriter;
	CAMBEFileReader*  m_ambeReader;
	CAMBEFileWriter*  m_ambeWriter;
	unsigned int      m_ambeBlockSize;

	enum RESP_TYPE {
		RESP_NONE,
		RESP_ERROR,
		RESP_RATEP,
		RESP_RATET,
		RESP_NAME,
		RESP_VERSION,
		RESP_AMBE,
		RESP_AUDIO,
		RESP_READY,
		RESP_UNKNOWN
	};

	void encodeIn(const float* audio, unsigned int length);
	bool encodeOut(unsigned char* ambe, unsigned int length);

	void decodeIn(const unsigned char* ambe, unsigned int length);
	bool decodeOut(float* audio, unsigned int length);

	RESP_TYPE getResponse(unsigned char* buffer, unsigned int length);
};

#endif
