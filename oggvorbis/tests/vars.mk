
ifneq ("$(with_oggvorbis)", "no")
TESTS+=$(patsubst %.cc,%,$(wildcard oggvorbis/tests/*.cc))
endif

oggvorbis/tests/oggvorbistest: LDLIBS+=libwvoggvorbis.a
oggvorbis/tests/oggvorbistest: LDFLAGS+=-logg -lvorbis -lvorbisenc
oggvorbis/tests/oggvorbistest: libwvoggvorbis.a

