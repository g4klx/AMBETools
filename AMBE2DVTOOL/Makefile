OBJECTS = AMBE2DVTOOL.o

.PHONY: all
all:		ambe2dvtool

ambe2dvtool:	$(OBJECTS) ../Common/Common.a
		$(CXX) $(OBJECTS) ../Common/Common.a $(LDFLAGS) $(LIBS) -o ambe2dvtool

-include $(OBJECTS:.o=.d)

%.o: %.cpp
		$(CXX) $(CFLAGS) -I../Common -c -o $@ $<
		$(CXX) -MM $(CFLAGS) -I../Common $< > $*.d

clean:
		$(RM) ambe2dvtool *.o *.d *.bak *~

install:
		install -m 755 ambe2dvtool /usr/local/bin

../Common/Common.a:
