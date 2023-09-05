/*
*   Copyright (C) 2017,2018,2019,2021.2023 by Jonathan Naylor G4KLX
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

#include "AMBE2DVTOOL.h"

#include "DVTOOLFileWriter.h"
#include "AMBEFileReader.h"
#include "Version.h"

#include <cstring>

// The length of the D-Star AMBE audio in one 20ms frame, including FEC
const unsigned int BUFFER_LENGTH = 9U;

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
	bool debug = false;

	int c;
	while ((c = ::getopt(argc, argv, "dv")) != -1) {
		switch (c) {
		case 'd':
			debug = true;
			break;
		case 'v':
			printf("Version: %s\n", version);
			return 0;
		case '?':
			break;
		default:
			fprintf(stderr, "Usage: AMBE2DVTOOL [-v] [-d] <input> <output>\n");
			break;
		}
	}

	if (optind > (argc - 2)) {
		fprintf(stderr, "Usage: AMBE2DVTOOL [-v] [-d] <input> <output>\n");
		return 1;
	}

	CAMBE2DVTOOL* ambe2dvtool = new CAMBE2DVTOOL(debug, std::string(argv[argc - 2]), std::string(argv[argc - 1]));

	int ret = ambe2dvtool->run();

	delete ambe2dvtool;

	return ret;
}

CAMBE2DVTOOL::CAMBE2DVTOOL(bool debug, const std::string& input, const std::string& output) :
m_debug(debug),
m_input(input),
m_output(output)
{
}

CAMBE2DVTOOL::~CAMBE2DVTOOL()
{
}

int CAMBE2DVTOOL::run()
{
	CAMBEFileReader reader(m_input, "");
	bool ret = reader.open();
	if (!ret)
		return 1;

	CDVTOOLFileWriter writer(m_output);
	ret = writer.open();
	if (!ret) {
		reader.close();
		return 1;
	}

	uint8_t buffer[BUFFER_LENGTH];
	unsigned int n = reader.read(buffer, BUFFER_LENGTH);

	while (n > 0U) {
		writer.write(buffer, n);

		n = reader.read(buffer, BUFFER_LENGTH);
	}

	writer.close();
	reader.close();

	return 0;
}
