
ifdef INCLUDE_OGGVORBIS
TESTS+=$(patsubst %.cc,%,$(wildcard oggvorbis/tests/*.cc))
endif

oggvorbis/tests/testoggvorbis-LIBS:=libwvoggvorbis.a -logg -lvorbis -lvorbisenc
