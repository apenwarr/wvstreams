_WIN32=_WIN32

XPATH+= $(WVSTREAMS)/Win32WvStreams \
	$(WVSTREAMS)/Win32WvStreams/libwvstreams \
	$(WVSTREAMS)/include

CC=ccache i586-mingw32msvc-gcc
CXX=ccache i586-mingw32msvc-g++
LD=i586-mingw32msvc-ld
AR=i586-mingw32msvc-ar

CFLAGS+=-include $(WVSTREAMS)/include/wvwin32-sanitize.h

