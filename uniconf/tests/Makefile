TOPDIR=../..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../../include

XPATH=../../utils ../../streams ../../configfile

TESTS=conftest itertest eventtest emutest

default: all

all: $(TESTS)

$(TESTS): uniconf.a

uniconf.a: ../uniconf.libs

LIBS = ${EFENCE}

clean:
	rm -f $(TESTS) *.new

