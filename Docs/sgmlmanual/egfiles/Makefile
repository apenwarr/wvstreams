#Remember to set TOPDIR to a directory containing wvrules.mk!
ifeq ($(TOPDIR),)
TOPDIR=../../..
WVSTREAMS_SRC=$(TOPDIR)
endif

STATIC=1

include $(TOPDIR)/wvrules.mk

LIBS=-L$(TOPDIR) -lwvstreams -lwvutils -lcrypto 
XPATH=$(TOPDIR)/include

LD_LIBRARY_PATH+=:$(TOPDIR)
export LD_LIBRARY_PATH

PROGS = $(shell echo *.cc | sed 's/\.cc//g')

default: all

all: $(PROGS)

$(PROGS): $(XFILES)

clean:
	rm -f $(PROGS)

