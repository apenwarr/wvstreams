#TOPDIR = ../..
include $(TOPDIR)/wvrules.mk

FOOBAR:=$(warning hey, I'm some crazy-ass recursive makefile!)

XPATH=$(TOPDIR)/src/wvstreams/utils $(TOPDIR)/src/wvstreams/streams \
      $(TOPDIR)/src/wvstreams/ipstreams $(TOPDIR)/src/wvstreams/crypto \
      $(TOPDIR)/src/wvstreams/configfile $(TOPDIR)/src/wvstreams/include \
      $(TOPDIR)/src/weaver/ $(TOPDIR)/src/ $(TOPDIR)/src/wvstreams/uniconf

LIBS=-L$(TOPDIR)/src/wvstreams

ifeq ($(XOR),)
  XOR=0
endif
CXXFLAGS += -DXOR=$(XOR)

all: uniconfdaemon

uniconfdaemon-LIBS+=-lwvstreams
uniconfdaemon: uniconfdaemon.o daemonmain.o uniconfdaemonconn.o

clean:
	rm -f core uniconfdaemon *.o

