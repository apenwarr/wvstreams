WVSTREAMS=..
include $(WVSTREAMS)/win32/wvrules-win32.mk

all: mytest.exe tcptest.exe

%.exe: %
	rm -f $@
	ln -s $(shell basename $<) $@
	
LIBS+=-lssl -lcrypto -lwsock32 -lgdi32 $(with_xplc)/libxplc.a -lsupc++

-lxplc -lxplc-cxx:

mytest tcptest resolvertest \
  ../ipstreams/tests/tcptest ../crypto/tests/ssltest: libwvutils.a

default: ../ipstreams/tests/tcptest ../crypto/tests/ssltest

DIRS=utils streams ipstreams uniconf crypto urlget configfile

_OBJFILES=$(call objects,$(addprefix ../,$(DIRS)))
OBJSKIP=fileutils.o strcrypt.o wvbdbhash.o wvdaemon.o wvdiriter.o \
	wvfork.o wvgzip.o wvipraw.o wvlockdev.o wvlockfile.o \
	wvlogfile.o wvmodem.o wvpipe.o wvqdbmhash.o wvshmzone.o \
	wvstreamsdaemon.o wvsubproc.o wvsyslog.o wvtask.o wvtest.o \
	wvunixsocket.o wvwatcher.o 
OBJFILES=$(filter-out $(addprefix %/,$(OBJSKIP)),$(_OBJFILES))

W=$(WVSTREAMS)/Win32WvStreams
libwvutils.a: $(call objects,$W/libwvutils $W/libwvstreams,../) $(OBJFILES)

clean:
	rm -f *test *.exe
