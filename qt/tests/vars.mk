
TESTS+=$(patsubst %.cc,%,$(wildcard qt/tests/*.cc))

%.moc: %.h
	@echo -n "Creating MOC file from $< ..."
	@moc $< -o $@
	@echo "Done."
        
qt/tests/nimon.o: qt/tests/nimon.moc

qt/tests/nimon-LIBS:=libwvqt.a -lqt -lkdeui -lkio

CPPFLAGS += -I/usr/include/kde
