
ifneq ("$(with_ogg)", "no")
  ifneq ("$(with_vorbis)", "no")
    TESTS+=$(patsubst %.cc,%,$(wildcard oggvorbis/tests/*.cc))
  endif
endif

oggvorbis/tests/oggvorbistest: LDLIBS+=libwvoggvorbis.a
oggvorbis/tests/oggvorbistest: LDFLAGS+=-logg -lvorbis -lvorbisenc
oggvorbis/tests/oggvorbistest: libwvoggvorbis.a

