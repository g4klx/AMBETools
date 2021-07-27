OBJECTS = AMBEFileReader.o AMBEFileWriter.o DV3000SerialController.o IMBEFEC.o SerialController.o Utils.o WAVFileReader.o WAVFileWriter.o \
	  codec2/codebooks.o codec2/codec2.o codec2/kiss_fft.o codec2/lpc.o codec2/nlp.o codec2/pack.o codec2/qbase.o codec2/quantise.o

.PHONY: all
all:		Common.a

Common.a:	$(OBJECTS)
		$(AR) rcs Common.a $(OBJECTS)

-include $(OBJECTS:.o=.d)

%.o: %.cpp
		$(CXX) $(CFLAGS) -c -o $@ $<
		$(CXX) -MM $(CFLAGS) $< > $*.d

.PHONY: clean
clean:
		$(RM) Common.a *.o *.d *.bak *~
