
ifneq ("$(with_qt)", "no")
QT_TESTS = $(patsubst %.cc,%,$(wildcard qt/tests/*.cc))
TESTS += $(QT_TESTS)
endif

QTT_OBJECTS = $(patsubst %.cc,%.o,$(wildcard qt/tests/*.cc))
$(QTT_OBJECTS): CPPFLAGS+=-I/usr/include/qt

$(QT_TESTS): libwvqt.a

qt/tests/qtstringtest-LIBS += -lqt
