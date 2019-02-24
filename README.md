These programs are designed to access an AMBE3000 vocoder connected to a serial port or a
USB port and to allow for the encoding and decoding of WAV files to and from
different AMBE formats.

IMBE is either handled via a DVSI USB3000-P25 or via the open source IMBE
vocoder. By default the open source vocoder is used, and the -p, -r and -s
options are not used.

There are two programs, AMBE2WAV and WAV2AMBE and their purposes are obvious from
their names. The usage of them is:

  ambe2wav [-v] [-a amplitude] [-g <signature>] [-m dstar|dmr|p25] [-f 0|1] [-p <port>] [-s <speed>] [-r] [-d] <input> <output>

  wav2ambe [-v] [-a amplitude] [-g <signature>] [-m dstar|dmr|p25] [-f 0|1] [-p <port>] [-s <speed>] [-r] [-d] <input> <output>

where

[-v] print the version and exit.

[-a amplitude] is the gain applied to the WAV file data, the default is 1.0

[-g signature] is an optional prefix at the beginning of the AMBE file

[-m dstar|dmr|p25] is the mode for which the AMBE will be generated. Note that P25 requires special hardware.

[-f 0|1] is whether FEC should be applied.

[-p <port>] is the serial port where the AMBE chip is attached, default is /dev/ttyUSB0

[-s <speed>] is the speed of the AMBE chip interface, default is 460800 baud

[-r] issue a reset at startup

[-d] print debugging information

On Linux these programs need access to the libsndfile library for compiling and running.

