
TESTS+=$(patsubst %.cc,%,$(wildcard oggvorbis/tests/*.cc))

oggvorbis/tests/testoggvorbis-LIBS:=libwvoggvorbis.a -logg -lvorbis -lvorbisenc
