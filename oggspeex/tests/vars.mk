
ifneq ("$(with_ogg)", "no")
  ifneq ("$(with_speex)", "no")
    TESTS+=$(patsubst %.cc,%,$(wildcard oggspeex/tests/*.cc))
  endif
endif

oggspeex/tests/oggspeextest: LDLIBS+=libwvoggspeex.a
oggspeex/tests/oggspeextest: LDFLAGS+=-logg -lspeex
oggspeex/tests/oggspeextest: libwvoggspeex.a

