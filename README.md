These programs are designed to access a DV3000 connected to a serial port or a
USB port and to allow for the encoding and decoding of WAV files to and from
different AMBE formats.

There are two programs, AMBE2WAV and WAV2AMBE and their purposes are obvious from
their names. The usage of them is:

ambe2wav [-a amplitude] [-g <signature>] [-m dstar|dmr|ysf|p25] [-f 0|1] [-p <port>] [-s <speed>] [-r] <input> <output>

wav2ambe [-a amplitude] [-g <signature>] [-m dstar|dmr|ysf|p25] [-f 0|1] [-p <port>] [-s <speed>] [-r] <input> <output>

where

[-a amplitude] is the gain applied to the WAV file data, the default is 1.0

[-g signature] is an optional prefix at the beginning of the AMBE file

[-m dstar|dmr|ysf|p25] is the mode for which the AMBE will be generated. Note that P25 requires special hardware.

[-f 0|1] is whether FEC should be applied. This only applies to DMR and P25

[-p <port>] is the serial port where the AMBE chip is attached, default is /dev/ttyUSB0

[-s <speed>] is the speed of the AMBE chip interface, default is 230400 baud

[-r] issue a reset at startup

This is work in progress.
