include ${TOPDIR}/wvrules.mk

XPATH=${TOPDIR}/src/wvstreams/include
unigremlin-LIBS = -L/home/mrwise/cvs/head/niti/src/wvstreams -lwvstreams -lwvutils -luniconf -lstdc++

all: unigremlin

unigremlin: unigremlin.o ../../libwvstreams.a ../../libwvutils.a \
	../../libuniconf.a
