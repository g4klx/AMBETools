/*
*   Copyright (C) 2017,2018,2019 by Jonathan Naylor G4KLX
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

#include "WAV2AMBE.h"

#include "WAVFileReader.h"
#include "AMBEFileWriter.h"
#include "Version.h"
#include "Utils.h"
#if !defined(HAVE_USB3000_P25)
#include "imbe_vocoder.h"
#include "IMBEFEC.h"
#endif

#include <cstring>

const uint8_t  BIT_MASK_TABLE8[]  = { 0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U };

#define WRITE_BIT8(p,i,b)   p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE8[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE8[(i)&7])
#define READ_BIT8(p,i)     (p[(i)>>3] & BIT_MASK_TABLE8[(i)&7])


#if defined(_WIN32) || defined(_WIN64)
char* optarg = NULL;
int optind = 1;

int getopt(int argc, char* const argv[], const char* optstring)
{
	if ((optind >= argc) || (argv[optind][0] != '-') || (argv[optind][0] == 0))
		return -1;

	int opt = argv[optind][1];
	const char *p = strchr(optstring, opt);

	if (p == NULL) {
		return '?';
	}

	if (p[1] == ':') {
		optind++;
		if (optind >= argc)
			return '?';

		optarg = argv[optind];
	}

	optind++;

	return opt;
}
#else
#include <unistd.h>
#endif

int main(int argc, char** argv)
{
	float amplitude = 1.0F;
	std::string signature;
	AMBE_MODE mode = MODE_DSTAR;
	bool fec = true;
	std::string port = "/dev/ttyUSB0";
	unsigned int speed = 460800U;
	bool reset = false;
	bool debug = false;

	int c;
	while ((c = ::getopt(argc, argv, "a:df:g:m:p:rs:v")) != -1) {
		switch (c) {
		case 'a':
			amplitude = float(::atof(optarg));
			break;
		case 'd':
			debug = true;
			break;
		case 'f':
			fec = ::atoi(optarg) != 0;
			break;
		case 'g':
			signature = std::string(optarg);
			break;
		case 'm':
			if (::strcmp(optarg, "dstar") == 0)
				mode = MODE_DSTAR;
			else if (::strcmp(optarg, "dmr") == 0)
				mode = MODE_DMR;
			else if (::strcmp(optarg, "p25") == 0)
				mode = MODE_P25;
			else
				mode = MODE_UNKNOWN;
			break;
		case 'p':
			port = std::string(optarg);
			break;
		case 'r':
			reset = true;
			break;
		case 's':
			speed = (unsigned int)::atoi(optarg);
			break;
		case 'v':
			printf("Version: %s\n", version);
			return 0;
		case '?':
			break;
		default:
			fprintf(stderr, "Usage: WAV2AMBE [-v] [-a amplitude] [-g <signature>] [-m dstar|dmr|p25] [-f 0|1] [-p <port>] [-s <speed>] [-r] [-d] <input> <output>\n");
			break;
		}
	}

	if (optind > (argc - 2)) {
		fprintf(stderr, "Usage: WAV2AMBE [-v] [-a amplitude] [-g <signature>] [-m dstar|dmr|p25] [-f 0|1] [-p <port>] [-s <speed>] [-r] [-d] <input> <output>\n");
		return 1;
	}

	if (mode == MODE_UNKNOWN) {
		::fprintf(stderr, "WAV2AMBE: unknown mode specified\n");
		return 1;
	}

	CWAV2AMBE* WAV2AMBE = new CWAV2AMBE(signature, mode, fec, port, speed, amplitude, reset, debug, std::string(argv[argc - 2]), std::string(argv[argc - 1]));

	int ret = WAV2AMBE->run();

	delete WAV2AMBE;

	return ret;
}

CWAV2AMBE::CWAV2AMBE(const std::string& signature, AMBE_MODE mode, bool fec, const std::string& port, unsigned int speed, float amplitude, bool reset, bool debug, const std::string& input, const std::string& output) :
m_signature(signature),
m_mode(mode),
m_fec(fec),
m_port(port),
m_speed(speed),
m_amplitude(amplitude),
m_reset(reset),
m_debug(debug),
m_input(input),
m_output(output)
{
}

CWAV2AMBE::~CWAV2AMBE()
{
}

int CWAV2AMBE::run()
{
	CWAVFileReader reader(m_input, AUDIO_BLOCK_SIZE);
	bool ret = reader.open();
	if (!ret)
		return 1;

	if (reader.getSampleRate() != AUDIO_SAMPLE_RATE) {
		::fprintf(stderr, "WAV2AMBE: input file has the wrong sample rate\n");
		reader.close();
		return 1;
	}

	if (reader.getChannels() > 1U) {
		::fprintf(stderr, "WAV2AMBE: input file has too many channels\n");
		reader.close();
		return 1;
	}

	CAMBEFileWriter writer(m_output, m_signature);
	ret = writer.open();
	if (!ret) {
		reader.close();
		return 1;
	}

#if !defined(HAVE_USB3000_P25)
	if (m_mode == MODE_P25) {
		printf("Using open source IMBE vocoder by Pavel Yazev\n");

		imbe_vocoder vocoder;
		unsigned int count = 0U;

		float audioFloat[AUDIO_BLOCK_SIZE];
		while (reader.read(audioFloat, AUDIO_BLOCK_SIZE) == AUDIO_BLOCK_SIZE) {
			int16_t audioInt[AUDIO_BLOCK_SIZE];
			for (unsigned int i = 0U; i < AUDIO_BLOCK_SIZE; i++)
				audioInt[i] = int16_t(audioFloat[i] * 3000.0F * m_amplitude + 0.5F);

			if (m_debug)
				CUtils::dump("encodeIn", (unsigned char*)audioInt, AUDIO_BLOCK_SIZE * sizeof(int16_t));

			int16_t frameInt[8U];
			vocoder.imbe_encode(frameInt, audioInt);

			unsigned char frame[11U];
			unsigned int offset = 0U;

			int16_t mask = 0x0800;
			for (unsigned int i = 0U; i < 12U; i++, mask >>= 1, offset++)
				WRITE_BIT8(frame, offset, (frameInt[0U] & mask) != 0);

			mask = 0x0800;
			for (unsigned int i = 0U; i < 12U; i++, mask >>= 1, offset++)
				WRITE_BIT8(frame, offset, (frameInt[1U] & mask) != 0);

			mask = 0x0800;
			for (unsigned int i = 0U; i < 12U; i++, mask >>= 1, offset++)
				WRITE_BIT8(frame, offset, (frameInt[2U] & mask) != 0);

			mask = 0x0800;
			for (unsigned int i = 0U; i < 12U; i++, mask >>= 1, offset++)
				WRITE_BIT8(frame, offset, (frameInt[3U] & mask) != 0);

			mask = 0x0400;
			for (unsigned int i = 0U; i < 11U; i++, mask >>= 1, offset++)
				WRITE_BIT8(frame, offset, (frameInt[4U] & mask) != 0);

			mask = 0x0400;
			for (unsigned int i = 0U; i < 11U; i++, mask >>= 1, offset++)
				WRITE_BIT8(frame, offset, (frameInt[5U] & mask) != 0);

			mask = 0x0400;
			for (unsigned int i = 0U; i < 11U; i++, mask >>= 1, offset++)
				WRITE_BIT8(frame, offset, (frameInt[6U] & mask) != 0);

			mask = 0x0040;
			for (unsigned int i = 0U; i < 7U; i++, mask >>= 1, offset++)
				WRITE_BIT8(frame, offset, (frameInt[7U] & mask) != 0);

			if (m_fec) {
				uint8_t data[18U];

				CIMBEFEC fec;
				fec.encode(data, frame);

				if (m_debug)
					CUtils::dump("encodeOut", data, 18U);

				writer.write(data, 18U);
			} else {
				if (m_debug)
					CUtils::dump("encodeOut", frame, 11U);

				writer.write(frame, 11U);
			}

			count++;
		}

		printf("Encoding: %u frames (%.2fs)\n", count, float(count) / 50.0F);
	} else {
#endif
		CDV3000SerialController controller(m_port, m_speed, m_mode, m_fec, m_amplitude, m_reset, m_debug, &reader, &writer);
		ret = controller.open();
		if (!ret) {
			writer.close();
			reader.close();
			return 1;
		}

		controller.process();

		controller.close();
#if !defined(HAVE_USB3000_P25)
	}
#endif

	writer.close();
	reader.close();

	return 0;
}
