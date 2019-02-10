OBJECTS = AMBE2WAV.o

.PHONY: all
all:		ambe2wav

ambe2wav:	$(OBJECTS) ../Common/Common.a
		$(CXX) $(OBJECTS) ../Common/Common.a $(LDFLAGS) $(LIBS) -o ambe2wav

-include $(OBJECTS:.o=.d)

%.o: %.cpp
		$(CXX) $(CFLAGS) -I../Common -c -o $@ $<
		$(CXX) -MM $(CFLAGS) -I../Common $< > $*.d

clean:
		$(RM) ambe2wav *.o *.d *.bak *~

../Common/Common.a:
