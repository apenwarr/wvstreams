#TOPDIR = ../..
include ../../../wvrules.mk

XPATH=$(TOPDIR)/src/wvstreams/utils $(TOPDIR)/src/wvstreams/streams \
      $(TOPDIR)/src/wvstreams/ipstreams $(TOPDIR)/src/wvstreams/crypto \
      $(TOPDIR)/src/wvstreams/configfile $(TOPDIR)/src/wvstreams/include \
      $(TOPDIR)/src/weaver/ $(TOPDIR)/src/ $(TOPDIR)/src/wvstreams/uniconf

ifeq ($(XOR),)
  XOR=0
endif
CXXFLAGS += -DXOR=$(XOR)

dyndnsd-LIBS += -lcrypto

all: uniconfdaemon

uniconfdaemon: uniconfdaemon.o daemonmain.o \
	 $(TOPDIR)/src/tunnelv/tunnelv.a \
         $(TOPDIR)/src/wvstreams/libwvcrypto.a \
         $(TOPDIR)/src/wvstreams/libwvstreams.a

convert_owners: convert_owners.o ../wvstreams/libwvstreams.a

clean:
	rm -f core uniconfdaemon *.o

