
ifneq ("$(with_qt)", "no")
TESTS+=$(patsubst %.cc,%,$(wildcard qt/tests/*.cc))
endif

qt/tests/qtstringtest: libwvqt.a
qt/tests/%: LDLIBS+=libwvqt.a
qt/tests/%: LDFLAGS+=-lqt
qt/tests/%: CPPFLAGS+=-I/usr/include/qt

