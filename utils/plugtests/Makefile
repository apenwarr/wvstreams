WVSTREAMS=../..
include $(WVSTREAMS)/wvrules.mk
XPATH=$(WVSTREAMS_INC)

all: myplugin.so myplugin plugtest

plugtest-LIBS=$(LIBWVBASE)

myplugin.so-LIBS=$(LIBUNICONF)
myplugin-LIBS=$(LIBUNICONF)

myplugin: myplugin.o empty.o

myplugin.so: myplugin.o

clean:
	rm -f *test myplugin
