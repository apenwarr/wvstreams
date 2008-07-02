
_WIN32=_WIN32

XPATH+= $(WVSTREAMS)/Win32WvStreams \
	$(WVSTREAMS)/Win32WvStreams/libwvstreams \
	$(WVSTREAMS)/Win32WvStreams/libuniconf \
	$(WVSTREAMS)/Win32WvStreams/cominclude \
	$(WVSTREAMS)/include

#CC=ccache i586-mingw32msvc-gcc
#CXX=ccache i586-mingw32msvc-g++
#LD=i586-mingw32msvc-ld
AR=i586-mingw32msvc-ar

# CFLAGS+=-include $(WVSTREAMS)/include/wvwin32-sanitize.h -Wno-unknown-pragmas

LIBS+=-lssl -lcrypto -lz -lole32 -lrpcrt4 -lwsock32 -lgdi32 -limagehlp \
	-lxplc-cxx -lxplc -lstdc++ -largp
