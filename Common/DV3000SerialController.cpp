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

#include "DV3000SerialController.h"
#include "Utils.h"

#include <cassert>

const unsigned char DV3000_START_BYTE   = 0x61U;

const unsigned char DV3000_TYPE_CONTROL = 0x00U;
const unsigned char DV3000_TYPE_AMBE    = 0x01U;
const unsigned char DV3000_TYPE_AUDIO   = 0x02U;

const unsigned char DV3000_CONTROL_RATET        = 0x09U;
const unsigned char DV3000_CONTROL_RATEP        = 0x0AU;
const unsigned char DV3000_CONTROL_PRODID       = 0x30U;
const unsigned char DV3000_CONTROL_RESETSOFTCFG = 0x34U;
const unsigned char DV3000_CONTROL_READY        = 0x39U;

const unsigned char DV3000_REQ_PRODID[]     = {DV3000_START_BYTE, 0x00U, 0x01U, DV3000_TYPE_CONTROL, DV3000_CONTROL_PRODID};
const unsigned int DV3000_REQ_PRODID_LEN    = 5U;

const unsigned char DV3000_REQ_RESET[] = {DV3000_START_BYTE, 0x00U, 0x07U, DV3000_TYPE_CONTROL, DV3000_CONTROL_RESETSOFTCFG, 0x05U, 0x00U, 0x00U, 0x0FU, 0x00U, 0x00U};
const unsigned int DV3000_REQ_RESET_LEN = 11U;

const unsigned char DV3000_REQ_DSTAR_FEC[]  = {DV3000_START_BYTE, 0x00U, 0x0DU, DV3000_TYPE_CONTROL, DV3000_CONTROL_RATEP, 0x01U, 0x30U, 0x07U, 0x63U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x48U};
const unsigned int DV3000_REQ_DSTAR_FEC_LEN = 17U;

const unsigned char DV3000_REQ_DSTAR_NOFEC[]  = {DV3000_START_BYTE, 0x00U, 0x0DU, DV3000_TYPE_CONTROL, DV3000_CONTROL_RATET, 0U};
const unsigned int DV3000_REQ_DSTAR_NOFEC_LEN = 6U;

const unsigned char DV3000_REQ_DMR_FEC[]    = { DV3000_START_BYTE, 0x00U, 0x02U, DV3000_TYPE_CONTROL, DV3000_CONTROL_RATET, 33U };
const unsigned int DV3000_REQ_DMR_FEC_LEN   = 6U;

const unsigned char DV3000_REQ_DMR_NOFEC[]  = { DV3000_START_BYTE, 0x00U, 0x02U, DV3000_TYPE_CONTROL, DV3000_CONTROL_RATET, 34U };
const unsigned int DV3000_REQ_DMR_NOFEC_LEN = 6U;

// const unsigned char DV3000_REQ_DMR[] = { DV3000_START_BYTE, 0x00U, 0x0DU, DV3000_TYPE_CONTROL, DV3000_CONTROL_RATEP, 0x04U, 0x31U, 0x07U, 0x54U, 0x24U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x6FU, 0x48U };
// const unsigned int DV3000_REQ_DMR_LEN = 17U;

/*
From: https://nw-digital-radio.groups.io/g/main/topic/2302628?p=Created,,,20,2,0,0

For Fusion V/D mode type 2 (a HR mode), I'm setting the AMBE-3000 chip to a custom rate using the these parameters listed in the chip data sheet ..

Total rate    Speech rate   FEC rate   RCW0    RCW1    RCW2    RCW3    RCW4    RCW5
2450           2450             0           0x0431   0x0754   0x0000   0x0000   0x0000   0x7031

And for Fusion FR mode I'm using ..

Total rate    Speech rate   FEC rate   RCW0    RCW1    RCW2    RCW3    RCW4    RCW5
7200           4400           2800        0x0458   0x0986   0x8020   0x0000   0x0000   0x7390

Well, their are 3 different rates Yaesu use ..

3600 .. 2450 voice .. 1150 fec .. V/D mode 1 .. index 33
2450 .. 2450 voice ..      0 fec ...V/D mode 2 .. index 34
7200 .. 4400 voice .. 2800 fec ..  FR mode ......index 59
all AMBE+2
*/
const unsigned char DV3000_REQ_YSF_FEC[] = { DV3000_START_BYTE, 0x00U, 0x0DU, DV3000_TYPE_CONTROL, DV3000_CONTROL_RATET, 59U };
const unsigned int DV3000_REQ_YSF_FEC_LEN = 6U;

const unsigned char DV3000_REQ_YSF_NOFEC[] = { DV3000_START_BYTE, 0x00U, 0x0DU, DV3000_TYPE_CONTROL, DV3000_CONTROL_RATET, 41U };
const unsigned int DV3000_REQ_YSF_NOFEC_LEN = 6U;

const unsigned char DV3000_REQ_P25_FEC[] = { DV3000_START_BYTE, 0x00U, 0x0DU, DV3000_TYPE_CONTROL, DV3000_CONTROL_RATEP, 0x05U, 0x58U, 0x08U, 0x6BU, 0x10U, 0x30U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U, 0x90U };
const unsigned int DV3000_REQ_P25_FEC_LEN = 17U;

const unsigned char DV3000_REQ_P25_NOFEC[] = { DV3000_START_BYTE, 0x00U, 0x0DU, DV3000_TYPE_CONTROL, DV3000_CONTROL_RATEP, 0x05U, 0x58U, 0x08U, 0x6BU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U, 0x58U };
const unsigned int DV3000_REQ_P25_NOFEC_LEN = 17U;

const unsigned char DV3000_AUDIO_HEADER[]   = {DV3000_START_BYTE, 0x01U, 0x42U, DV3000_TYPE_AUDIO, 0x00U, 0xA0U};
const unsigned char DV3000_AUDIO_HEADER_LEN = 6U;

const unsigned char DV3000_AMBE_HEADER[]    = {DV3000_START_BYTE, 0x00U, 0x0BU, DV3000_TYPE_AMBE, 0x01U, 0x48U};
const unsigned char DV3000_AMBE_HEADER_LEN  = 6U;

const unsigned int DV3000_HEADER_LEN = 4U;

const unsigned int BUFFER_LENGTH = 400U;

CDV3000SerialController::CDV3000SerialController(const std::string& device, unsigned int speed, AMBE_MODE mode, bool fec, float amplitude, bool reset, CWAVFileReader* reader, CAMBEFileWriter* writer) :
m_serial(device, SERIAL_SPEED(speed)),
m_mode(mode),
m_fec(fec),
m_amplitude(amplitude),
m_reset(reset),
m_direction(AMBE_ENCODING),
m_wavReader(reader),
m_wavWriter(NULL),
m_ambeReader(NULL),
m_ambeWriter(writer),
m_ambeBlockSize(0U),
m_inCount(0U),
m_outCount(0U)
{
	assert(reader != NULL);
	assert(writer != NULL);
}

CDV3000SerialController::CDV3000SerialController(const std::string& device, unsigned int speed, AMBE_MODE mode, bool fec, float amplitude, bool reset, CAMBEFileReader* reader, CWAVFileWriter* writer) :
m_serial(device, SERIAL_SPEED(speed)),
m_mode(mode),
m_fec(fec),
m_amplitude(amplitude),
m_reset(reset),
m_direction(AMBE_DECODING),
m_wavReader(NULL),
m_wavWriter(writer),
m_ambeReader(reader),
m_ambeWriter(NULL),
m_ambeBlockSize(0U),
m_inCount(0U),
m_outCount(0U)
{
	assert(reader != NULL);
	assert(writer != NULL);
}

CDV3000SerialController::~CDV3000SerialController()
{
}

bool CDV3000SerialController::open()
{
	bool res = m_serial.open();
	if (!res)
		return false;

	unsigned char buffer[BUFFER_LENGTH];

	if (m_reset) {
		m_serial.write(DV3000_REQ_RESET, DV3000_REQ_RESET_LEN);

		RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);
		while (resp == RESP_NONE) {
			CUtils::sleep(10U);
			type = getResponse(buffer, BUFFER_LENGTH);
		}

		if (type != RESP_READY) {
			m_serial.close();
			return false;
		}
	}

	m_serial.write(DV3000_REQ_PRODID, DV3000_REQ_PRODID_LEN);

	RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);
	while (resp == RESP_NONE) {
		CUtils::sleep(10U);
		type = getResponse(buffer, BUFFER_LENGTH);
	}

	if (type == RESP_NAME) {
		std::string name((char*)(buffer + 5U));
		::fprintf(stdout, "DV3000 chip identified as: %s\n", name.c_str());
	} else {
		m_serial.close();
		return false;
	}

	if (m_mode == MODE_DSTAR && m_fec) {
		m_serial.write(DV3000_REQ_DSTAR_FEC, DV3000_REQ_DSTAR_FEC_LEN);
		// CUtils::dump("Configure D-Star + FEC", DV3000_REQ_DSTAR_FEC, DV3000_REQ_DSTAR_FEC_LEN);
		m_ambeBlockSize = 9U;
	} else if (m_mode == MODE_DSTAR && !m_fec) {
		m_serial.write(DV3000_REQ_DSTAR_NOFEC, DV3000_REQ_DSTAR_NOFEC_LEN);
		// CUtils::dump("Configure D-Star", DV3000_REQ_DSTAR_NOFEC, DV3000_REQ_DSTAR_NOFEC_LEN);
		m_ambeBlockSize = 6U;
	} else if (m_mode == MODE_DMR && m_fec) {
		m_serial.write(DV3000_REQ_DMR_FEC, DV3000_REQ_DMR_FEC_LEN);
		// CUtils::dump("Configure DMR + FEC", DV3000_REQ_DMR_FEC, DV3000_REQ_DMR_FEC_LEN);
		m_ambeBlockSize = 9U;
	} else if (m_mode == MODE_DMR && !m_fec) {
		m_serial.write(DV3000_REQ_DMR_NOFEC, DV3000_REQ_DMR_NOFEC_LEN);
		// CUtils::dump("Configure DMR", DV3000_REQ_DMR_NOFEC, DV3000_REQ_DMR_NOFEC_LEN);
		m_ambeBlockSize = 7U;
	} else if (m_mode == MODE_YSF && m_fec) {
		m_serial.write(DV3000_REQ_YSF_FEC, DV3000_REQ_YSF_FEC_LEN);
		// CUtils::dump("Configure YSF + FEC", DV3000_REQ_YSF_FEC, DV3000_REQ_YSF_FEC_LEN);
		m_ambeBlockSize = 18U;
	} else if (m_mode == MODE_YSF && !m_fec) {
		m_serial.write(DV3000_REQ_YSF_NOFEC, DV3000_REQ_YSF_NOFEC_LEN);
		// CUtils::dump("Configure YSF", DV3000_REQ_YSF_NOFEC, DV3000_REQ_YSF_NOFEC_LEN);
		m_ambeBlockSize = 11U;
	} else if (m_mode == MODE_P25 && m_fec) {
		m_serial.write(DV3000_REQ_P25_FEC, DV3000_REQ_P25_FEC_LEN);
		// CUtils::dump("Configure P25 + FEC", DV3000_REQ_P25_FEC, DV3000_REQ_P25_FEC_LEN);
		m_ambeBlockSize = 18U;
	} else if (m_mode == MODE_P25 && !m_fec) {
		m_serial.write(DV3000_REQ_P25_NOFEC, DV3000_REQ_P25_NOFEC_LEN);
		// CUtils::dump("Configure P25", DV3000_REQ_P25_NOFEC, DV3000_REQ_P25_NOFEC_LEN);
		m_ambeBlockSize = 11U;
	} else {
		return false;
	}

	type = getResponse(buffer, BUFFER_LENGTH);
	while (resp == RESP_NONE) {
		CUtils::sleep(10U);
		type = getResponse(buffer, BUFFER_LENGTH);
	}

	if (type != RESP_ERROR && type != RESP_RATET) {
		m_serial.close();
		return false;
	}

	return true;
}

void CDV3000SerialController::process()
{
	if (m_direction == AMBE_ENCODING) {
		assert(m_wavReader != NULL);
		assert(m_ambeWriter != NULL);

		do {
			unsigned int outstanding = m_inCount - m_outCount;
			if (outstanding < 4U) {
				float audio[AUDIO_BLOCK_SIZE];
				if (m_wavReader->read(audio, AUDIO_BLOCK_SIZE) == AUDIO_BLOCK_SIZE) {
					encodeIn(audio, AUDIO_BLOCK_SIZE);
					m_inCount++;
				}
			}

			CUtils::sleep(10U);

			unsigned char ambe[25U];
			if (encodeOut(ambe, m_ambeBlockSize)) {
				m_ambeWriter->write(ambe, m_ambeBlockSize);
				m_outCount++;
			}
		} while (m_inCount != m_outCount);
	} else {
		assert(m_ambeReader != NULL);
		assert(m_wavWriter != NULL);

		do {
			unsigned int outstanding = m_inCount - m_outCount;
			if (outstanding < 4U) {
				unsigned char ambe[25U];
				if (m_ambeReader->read(ambe, m_ambeBlockSize) == m_ambeBlockSize) {
					decodeIn(ambe, m_ambeBlockSize);
					m_inCount++;
				}
			}

			CUtils::sleep(10U);

			float audio[AUDIO_BLOCK_SIZE];
			if (decodeOut(audio, AUDIO_BLOCK_SIZE)) {
				m_wavWriter->write(audio, AUDIO_BLOCK_SIZE);
				m_outCount++;
			}
		} while (m_inCount != m_outCount);
	}
}

void CDV3000SerialController::encodeIn(const float* audio, unsigned int length)
{
	assert(audio != NULL);

	unsigned char buffer[DV3000_AUDIO_HEADER_LEN + AUDIO_BLOCK_SIZE * 2U];
	::memcpy(buffer, DV3000_AUDIO_HEADER, DV3000_AUDIO_HEADER_LEN);

	int8_t* q = (int8_t*)(buffer + DV3000_AUDIO_HEADER_LEN);
	for (unsigned int i = 0; i < AUDIO_BLOCK_SIZE; i++, q += 2U) {
		int16_t word = int16_t(audio[i] * m_amplitude * 32767.0F);

		q[0U] = (word & 0xFF00) >> 8;
		q[1U] = (word & 0x00FF) >> 0;
	}

	m_serial.write(buffer, DV3000_AUDIO_HEADER_LEN + AUDIO_BLOCK_SIZE * 2U);
}

bool CDV3000SerialController::encodeOut(unsigned char* ambe, unsigned int length)
{
	assert(ambe != NULL);

	unsigned char buffer[BUFFER_LENGTH];
	RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);
	if (type != RESP_AMBE)
		return false;

	::memcpy(ambe, buffer + DV3000_AMBE_HEADER_LEN, m_ambeBlockSize);

	return true;
}

void CDV3000SerialController::decodeIn(const unsigned char* ambe, unsigned int length)
{
	assert(ambe != NULL);

	unsigned char buffer[DV3000_AMBE_HEADER_LEN + 25U];
	::memcpy(buffer, DV3000_AMBE_HEADER, DV3000_AMBE_HEADER_LEN);

	::memcpy(buffer + DV3000_AMBE_HEADER_LEN, ambe, m_ambeBlockSize);

	m_serial.write(buffer, DV3000_AMBE_HEADER_LEN + m_ambeBlockSize);
}

bool CDV3000SerialController::decodeOut(float* audio, unsigned int length)
{
	assert(audio != NULL);

	unsigned char buffer[BUFFER_LENGTH];
	RESP_TYPE type = getResponse(buffer, BUFFER_LENGTH);
	if (type != RESP_AUDIO)
		return false;

	uint8_t* q = (uint8_t*)(buffer + DV3000_AUDIO_HEADER_LEN);
	for (unsigned int i = 0U; i < AUDIO_BLOCK_SIZE; i++, q += 2U) {
		int16_t word = (q[0] << 8) | (q[1U] << 0);

		audio[i] = float(word) * m_amplitude / 32768.0F;
	}

	return true;
}

void CDV3000SerialController::close()
{
	m_serial.close();
}

CDV3000SerialController::RESP_TYPE CDV3000SerialController::getResponse(unsigned char* buffer, unsigned int length)
{
	assert(buffer != NULL);
	assert(length >= BUFFER_LENGTH);

	int len = m_serial.read(buffer, 1U);
	if (len < 0)
		return RESP_ERROR;
	else if (len == 0)
		return RESP_NONE;

	if (buffer[0U] != DV3000_START_BYTE)
		return RESP_NONE;

	unsigned int offset = 1U;

	while (offset < 3U) {
		int len = m_serial.read(buffer + offset, 3U - offset);
		if (len < 0)
			return RESP_ERROR;
		else if (len == 0)
			CUtils::sleep(10U);
		else
			offset += len;
	}

	unsigned int respLen = (buffer[1U] & 0x0FU) * 256U + buffer[2U] + DV3000_HEADER_LEN;

	while (offset < respLen) {
		int len = m_serial.read(buffer + offset, respLen - offset);
		if (len < 0)
			return RESP_ERROR;
		else if (len == 0)
			CUtils::sleep(10U);
		else
			offset += len;
	}

	if (buffer[3U] == DV3000_TYPE_AUDIO) {
		return RESP_AUDIO;
	} else if (buffer[3U] == DV3000_TYPE_AMBE) {
		return RESP_AMBE;
	} else if (buffer[3U] == DV3000_TYPE_CONTROL) {
		if (buffer[4U] == DV3000_CONTROL_PRODID) {
			return RESP_NAME;
		} else if (buffer[4U] == DV3000_CONTROL_RATEP) {
			return RESP_RATEP;
		} else if (buffer[4U] == DV3000_CONTROL_RATET) {
			return RESP_RATET;
		} else if (buffer[4U] == DV3000_CONTROL_READY) {
			return RESP_READY;
		} else {
			CUtils::dump("Unknown control data", buffer, respLen);
			return RESP_UNKNOWN;
		}
	} else {
		CUtils::dump("Unknown data", buffer, respLen);
		return RESP_UNKNOWN;
	}
}
