OBJECTS = WAV2AMBE.o

.PHONY: all
all:		wav2ambe

wav2ambe:	$(OBJECTS) ../Common/Common.a
		$(CXX) $(OBJECTS) ../Common/Common.a $(LDFLAGS) $(LIBS) -o wav2ambe

-include $(OBJECTS:.o=.d)

%.o: %.cpp
		$(CXX) $(CFLAGS) -I../Common -c -o $@ $<
		$(CXX) -MM $(CFLAGS) -I../Common $< > $*.d

install:
		install -m 755 wav2ambe /usr/local/bin

clean:
		$(RM) wav2ambe *.o *.d *.bak *~

../Common/Common.a:
