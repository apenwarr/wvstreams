
libwvqt.so-OBJECTS += $(patsubst %.cc,%.o,$(wildcard qt/*.cc))

$(libwvqt.so-OBJECTS): CPPFLAGS+=-I/usr/include/qt

qt/wvqtstreamclone.o: include/wvqtstreamclone.moc

include $(call doinclude,vars.mk,qt)

