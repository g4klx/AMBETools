export CXX     := g++
export CFLAGS  := -O2 -Wall -I../../imbe_vocoder/src/lib
export LDFLAGS := 
export LIBS    := -lsndfile ../../imbe_vocoder/src/lib/imbe.a

all:	AMBE2WAV/ambe2wav WAV2AMBE/wav2ambe

AMBE2WAV/ambe2wav:	Common/Common.a force
	$(MAKE) -C AMBE2WAV

WAV2AMBE/wav2ambe:	Common/Common.a force
	$(MAKE) -C WAV2AMBE

Common/Common.a: force
	$(MAKE) -C Common

.PHONY: clean
clean:
	$(MAKE) -C Common clean
	$(MAKE) -C AMBE2WAV clean
	$(MAKE) -C WAV2AMBE clean

.PHONY: force
force:
	@true
