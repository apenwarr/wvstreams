
libwvqt.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard qt/*.cc))

qt/wvqtstreamclone.moc: include/wvqtstreamclone.h
	@echo -n "Creating MOC file from $< ..."
	@moc $< -o $@
	@echo "Done."

qt/wvqtstreamclone.o: qt/wvqtstreamclone.moc

CPPFLAGS += -I/usr/include/qt

include $(call doinclude,vars.mk,qt)

