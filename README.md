These programs are designed to access an AMBE3000 vocoder connected to a serial port or a
USB port and to allow for the encoding and decoding of WAV files to and from
different AMBE formats.

IMBE is either handled via a DVSI USB3000-P25 or via the open source IMBE
vocoder. By default the open source vocoder is used, and the -p, -r and -s
options not being used.

Codec2 is handled by a built-in codec 2 vocoder and the two audio formats used for M17 are included. The
vocoder is included within this project. The -f, -p, -r, and -s options are not used.

There are three programs, AMBE2WAV, WAV2AMBE, and AMBE2DVTOOL and their purposes are obvious from
their names. The usage of them is:

  ambe2wav [-v] [-a amplitude] [-g <signature>] [-m dstar|dmr|p25|nxdn|m17-3200|m17-1600] [-f 0|1] [-p <port>] [-s <speed>] [-r] [-d] <input> <output>

  wav2ambe [-v] [-a amplitude] [-g <signature>] [-m dstar|dmr|p25|nxdn|m17-3200|m17-1600] [-f 0|1] [-p <port>] [-s <speed>] [-r] [-d] <input> <output>

  ambe2dvtool [-v] [-g <signature>] [-d] <input> <output>

where

[-v] print the version and exit.

[-a amplitude] is the gain applied to the WAV file data, the default is 1.0

[-g signature] is an optional prefix at the beginning of the AMBE/IMBE/Codec2 file

[-m dstar|dmr|p25|nxdn|m17-3200|m17-1600] is the mode for which the AMBE/IMBE/Codec2 will be generated. Note that P25 requires special hardware.

[-f 0|1] is whether FEC should be applied.

[-p <port>] is the serial port where the AMBE chip is attached, default is /dev/ttyUSB0

[-s <speed>] is the speed of the AMBE chip interface, default is 460800 baud

[-r] issue a reset at startup

[-d] print debugging information


## Building

The repo https://github.com/g4klx/imbe_vocoder needs to be cloned and placed at the same level in the file structure as the folder for this repository

On Linux these programs need access to the libsndfile library for compiling and running.

