
ifneq ("$(with_oggvorbis)", "no")
TESTS+=$(patsubst %.cc,%,$(wildcard oggvorbis/tests/*.cc))
endif

oggvorbis/tests/testoggvorbis-LIBS:=libwvoggvorbis.a \
    libwvstreams.a libwvutils.a -logg -lvorbis -lvorbisenc
