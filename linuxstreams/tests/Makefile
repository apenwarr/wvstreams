TOPDIR=../..
include $(TOPDIR)/wvrules.mk

CXXFLAGS += -I../../include

XPATH=../.. ../../utils ../../streams ../../ipstreams

TESTS=aliastest routetest

default: all

all: $(TESTS)

ipstreams.a: ../../ipstreams/ipstreams.libs

#LIBS=$(EFENCE)
$(TESTS): ipstreams.a


clean:
	rm -f $(TESTS)

