OBJECTS = AMBEFileReader.o AMBEFileWriter.o DV3000SerialController.o \
	IMBEFEC.o SerialController.o Utils.o WAVFileReader.o WAVFileWriter.o

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
