
ifneq ("$(with_qt)", "no")
TESTS+=$(patsubst %.cc,%,$(wildcard qt/tests/*.cc))
endif

qt/tests/qtstringtest: LDLIBS+=libwvqt.a
qt/tests/qtstringtest: LDFLAGS+=-lqt

